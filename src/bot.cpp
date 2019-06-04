#include <map>

#include <boost/bind.hpp>
#include <boost/log/trivial.hpp>

#include "./bot.h"
#include "./command.h"
#include "./opcode.h"
#include "./event.h"

using namespace boost::asio;
using namespace boost::beast;

namespace
{

auto generate_gateway_option(uint32_t const& version_gateway) -> std::string
{
    return "/?v=" + std::to_string(version_gateway) + "&encoding=json";
}

using key = std::decay_t< qyzk::ohno::config::cache_type >::key_type;

auto get_sequence(qyzk::ohno::config const& config) -> uint32_t
{
    auto const& cache = config.get_cache();
    if (!cache.has< key::last_event_sequence >())
        return 0;
    else 
        return cache.get< key::last_event_sequence >();
}

} // namespace

namespace qyzk::ohno
{

using json = nlohmann::json;

bot::bot(
    std::filesystem::path const& path_config,
    boost::asio::io_context& context_io,
    boost::asio::ssl::context& context_ssl,
    ohno::config& config,
    get_gateway_bot_result const& material_bot)
    : m_path_config(path_config)
    , m_config(config)
    , m_context_io(context_io)
    , m_context_ssl(context_ssl)
    , m_buffer_event()
    , m_stream_gateway(
        connect_to_gateway(
            m_context_io,
            m_context_ssl,
            material_bot.url,
            generate_gateway_option(m_config.get_gateway_version())))
    , m_timer_heartbeat(m_context_io)
    , m_status_connection(connection_status_type::connecting)
    , m_is_running(true)
{
}

auto bot::async_listen_event(void) -> void
{
    m_stream_gateway.async_read(
        m_buffer_event,
        boost::bind(
            &bot::handle_event,
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

auto bot::stop(void) -> void
{
    m_is_running = false;
    m_timer_heartbeat.cancel();

    error_code error;
    m_stream_gateway.close(websocket::close_code::going_away, error);
    if (error)
    {
        if (error == boost::asio::ssl::error::stream_truncated)
        {
            BOOST_LOG_TRIVIAL(debug) << "server has closed connection";
        }
        else
        {
            BOOST_LOG_TRIVIAL(error) << "oh no fatal error on closing connection: " << error.message();
        }
    }
}

auto bot::handle_event(
    error_code const& error,
    std::size_t const bytes_written)
    -> void
{
    if (error)
    {
        if (error == boost::asio::error::operation_aborted)
            BOOST_LOG_TRIVIAL(debug) << "listening event operation has been aborted";
        else
            BOOST_LOG_TRIVIAL(error) << "oh no failed to listen gateway event: " << error.message();
        return;
    }

    auto buffer_string = buffers_to_string(m_buffer_event.data());
    m_buffer_event.clear();
    BOOST_LOG_TRIVIAL(debug) << "read event: " << buffer_string;

    auto payload = json::parse(buffer_string);
    auto const opcode = static_cast< opcode_type >(payload["op"]);
    auto const& opcode_name = opcode_names.find(opcode)->second;
    BOOST_LOG_TRIVIAL(debug) << "opcode: " << opcode_name;

    auto& cache = m_config.get_cache();

    switch (opcode)
    {
    case opcode_type::dispatch:
        handle_event_dispatch(payload);
        break;

    case opcode_type::heartbeat:
        BOOST_LOG_TRIVIAL(debug) << "get heartbeat event";
        ohno::heartbeat(m_stream_gateway, get_sequence(m_config));
        break;

    case opcode_type::hello:
        BOOST_LOG_TRIVIAL(debug) << "get hello event";
        m_interval_heartbeat = payload["d"]["heartbeat_interval"];
        m_timer_heartbeat.expires_after(
            chrono::milliseconds(m_interval_heartbeat));
        m_timer_heartbeat.async_wait(
            boost::bind(
                &bot::heartbeat,
                this,
                placeholders::error));
        ohno::heartbeat(m_stream_gateway, get_sequence(m_config));
        if (cache.has< key::session_id >())
        {
            BOOST_LOG_TRIVIAL(debug) << "resuming previous session";
            resume(
                m_stream_gateway,
                m_config.get_token(),
                cache.get< key::session_id >(),
                get_sequence(m_config));
            m_status_connection = connection_status_type::resuming;
        }
        else
        {
            BOOST_LOG_TRIVIAL(debug) << "starting new session";
            ohno::identify(m_stream_gateway, m_config.get_token());
            m_status_connection = connection_status_type::connecting;
        }
        break;

    case opcode_type::invalid_session:
        handle_invalid_session(payload);
        break;

    default:
        BOOST_LOG_TRIVIAL(debug) << "skipped handling event " << opcode_name;
    }

    if (m_is_running)
        async_listen_event();
}

auto bot::async_heartbeat() -> void
{
    m_timer_heartbeat.expires_at(
        m_timer_heartbeat.expiry()
        + chrono::milliseconds(m_interval_heartbeat));
    m_timer_heartbeat.async_wait(
        boost::bind(
            &bot::heartbeat,
            this,
            placeholders::error));
}

auto bot::heartbeat(error_code const& error) -> void
{
    if (error)
    {
        if (error == boost::asio::error::operation_aborted)
            BOOST_LOG_TRIVIAL(debug) << "heartbeat timer operation aborted";
        else
            BOOST_LOG_TRIVIAL(error) << "error occured during heartbeat timer operation: " << error.message();
        return;
    }

    auto const sequence = get_sequence(m_config);
    ohno::heartbeat(m_stream_gateway, sequence);
    BOOST_LOG_TRIVIAL(debug) << "sent heartbeat ping, sequence: " << sequence;

    if (m_is_running)
        async_heartbeat();
}

auto bot::handle_invalid_session(json const& payload) -> void
{
    auto& cache = m_config.get_cache();

    switch (m_status_connection)
    {
    case connection_status_type::connecting:
        BOOST_LOG_TRIVIAL(error) << "starting new session has rejected, maybe rate limited?";
        return;

    case connection_status_type::connected:
        BOOST_LOG_TRIVIAL(debug) << "current session has been invaliated";
        if (static_cast< bool >(payload["d"]))
        {
            BOOST_LOG_TRIVIAL(debug) << "resuming is availble, resuming session";
            m_status_connection = connection_status_type::resuming;
            resume(m_stream_gateway, m_config.get_token(), cache.get< key::session_id >(), get_sequence(m_config));
        }
        else
        {
            BOOST_LOG_TRIVIAL(debug) << "resuming is unavailable, starting new session";
            steady_timer timer(m_context_io);
            timer.expires_after(chrono::seconds(2));
            timer.wait();
            m_status_connection = connection_status_type::connecting;
            identify(m_stream_gateway, m_config.get_token());
        }
        break;

    case connection_status_type::resuming:
        BOOST_LOG_TRIVIAL(debug) << "resuming has been rejected, starting new session";
        m_status_connection = connection_status_type::connecting;
        identify(m_stream_gateway, m_config.get_token());
        break;
    }
}

auto bot::handle_event_dispatch(json const& payload) -> void
{
    auto& cache = m_config.get_cache();
    cache.set< key::last_event_sequence >(payload["s"]);

    auto const event_name = payload["t"];
    auto const event = events.find(event_name)->second;
    auto const& data = payload["d"];

    switch (event)
    {
    case event_type::ready:
        BOOST_LOG_TRIVIAL(debug) << "get ready event";
        m_status_connection = connection_status_type::connected;
        cache.set< key::session_id >(data["session_id"]);
        break;

    case event_type::resumed:
        BOOST_LOG_TRIVIAL(debug) << "get resumed event";
        m_status_connection = connection_status_type::connected;
        break;

    default:
        BOOST_LOG_TRIVIAL(debug) << "skipping event " << event_name;
    }

    save_config(m_path_config, m_config);
}

} // namespace qyzk::ohno
