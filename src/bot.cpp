#include <boost/bind.hpp>
#include <boost/log/trivial.hpp>

#include "./bot.h"

using namespace boost::asio;
using namespace boost::beast;

namespace
{

auto handle_websocket_close(error_code const& error) -> void
{
    if (error)
    {
        BOOST_LOG_TRIVIAL(warning) << "error occured during closing websocket: " << error.message();
    }
    BOOST_LOG_TRIVIAL(debug) << "closed websocket";
}

enum class opcode_type
{
    dispatch = 0,
    heartbeat = 1,
    identify = 2,
    resume = 6,
    invalid_session = 9,
    hello = 10,
    hello_ack = 11,
};

enum class event_type
{
    ready = 0,
    guild_create = 1,
    resumed = 2,
    message_create = 3,
    unknown,
};

auto to_event_type(std::string const& event_name) -> event_type
{
    if (event_name == "READY")
        return event_type::ready;
    if (event_name == "GUILD_CREATE")
        return event_type::guild_create;
    if (event_name == "RESUMED")
        return event_type::resumed;
    if (event_name == "MESSAGE_CREATE")
        return event_type::message_create;
    return event_type::unknown;
}

auto heartbeat(qyzk::ohno::bot& bot, error_code const& error) -> void
{
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "error occured waiting async timer: " << error.message();
        bot.stop();
        return;
    }

    if (!bot.is_running())
        return;

    bot.beat();

    auto& timer = bot.get_heartbeat_timer();
    timer.expires_at(
        timer.expiry() + chrono::milliseconds(bot.get_heartbeat_interval()));
    timer.async_wait(
        boost::bind(
            heartbeat,
            std::ref(bot),
            placeholders::error));
}

auto start_heartbeat(qyzk::ohno::bot& bot)
{
    auto& timer = bot.get_heartbeat_timer();
    timer.expires_after(chrono::milliseconds(bot.get_heartbeat_interval()));
    timer.async_wait(
        boost::bind(
            heartbeat,
            std::ref(bot),
            placeholders::error));
}

auto identify(
    qyzk::ohno::bot& bot,
    error_code error)
    -> void
{
    nlohmann::json properties;
    properties["$os"] = "Archlinux";
    properties["$browser"] = "oh no bot";
    properties["$device"] = "oh no bot";

    nlohmann::json presence;
    presence["status"] = "online";
    presence["afk"] = false;

    nlohmann::json data;
    data["token"] = bot.get_config().get_token();
    data["properties"] = properties;
    data["presence"] = presence;

    nlohmann::json payload;
    payload["op"] = opcode_type::identify;
    payload["d"] = data;

    bot.get_websocket().write(boost::asio::buffer(payload.dump()), error);
}

auto resume(
    qyzk::ohno::bot& bot,
    error_code& error)
{
    auto const& config = bot.get_config();

    nlohmann::json data;
    data["token"] = config.get_token();
    data["session_id"] = config.get_session_id();
    data["seq"] = config.get_event_sequence();

    nlohmann::json payload;
    payload["op"] = opcode_type::resume;
    payload["d"] = data;

    bot.get_websocket().write(boost::asio::buffer(payload.dump()), error);
}

auto handle_message(qyzk::ohno::bot& bot, nlohmann::json const& payload) -> void
{
    auto const message = payload["d"];
    auto const content = message["content"];
    auto const user = message["author"]["id"];

    if (user == "257451263820562433")
    {
        auto const channel_id = message["channel_id"];
    }
}

auto handle_event_dispatch(qyzk::ohno::bot& bot, nlohmann::json const& payload) -> void
{
    auto& config = bot.get_config();
    config.set_event_sequence(payload["s"]);
    BOOST_LOG_TRIVIAL(debug) << "event sequence: " << config.get_event_sequence();

    auto event_name = payload["t"];
    BOOST_LOG_TRIVIAL(debug) << "event type is " << event_name;

    auto const event_type_value = to_event_type(event_name);

    // discard missed events
    if (bot.is_resuming() && event_type_value != event_type::resumed)
    {
        BOOST_LOG_TRIVIAL(debug) << "discarded missed event";
        return;
    }

    switch (event_type_value)
    {
    case event_type::ready:
        config.set_session_id(payload["d"]["session_id"]);
        BOOST_LOG_TRIVIAL(debug) << "new session ID is: " << config.get_session_id();
        break;

    case event_type::guild_create:
        BOOST_LOG_TRIVIAL(debug) << "guild created";
        break;

    case event_type::resumed:
        bot.stop_resuming();
        BOOST_LOG_TRIVIAL(debug) << "resuming completed";
        break;

    case event_type::message_create:
        handle_message(bot, payload);
        break;

    case event_type::unknown:
        BOOST_LOG_TRIVIAL(debug) << "unhandled event dispatch";
        break;
    }

    config.save_cache();
}

auto handle_event(
    qyzk::ohno::bot& bot,
    error_code const& error,
    std::size_t bytes_written)
    -> void
{
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "error occured async read on websocket: " << error.message();
        bot.stop();
        return;
    }

    auto& buffer = bot.get_websocket_buffer();
    auto payload_string = buffers_to_string(buffer.data());
    buffer.clear();
    auto payload = nlohmann::json::parse(payload_string);

    BOOST_LOG_TRIVIAL(debug) << "read: " << payload_string;

    error_code error_write;
    auto const& config = bot.get_config();

    auto const opcode = static_cast< opcode_type >(payload["op"]);
    switch (opcode)
    {
    case opcode_type::dispatch:
        BOOST_LOG_TRIVIAL(debug) << "received event dispatch(0)";
        handle_event_dispatch(bot, payload);
        break;

    case opcode_type::heartbeat:
        BOOST_LOG_TRIVIAL(debug) << "received heartbeat ping request(1)";
        bot.beat();
        break;

    case opcode_type::hello:
        BOOST_LOG_TRIVIAL(debug) << "received event hello(10)";
        bot.set_heartbeat_interval(payload["d"]["heartbeat_interval"]);
        BOOST_LOG_TRIVIAL(debug) << "heartbeat interval is " << bot.get_heartbeat_interval() << " milliseconds";
        bot.beat();
        start_heartbeat(bot);

        if (config.has_session_id())
        {
            BOOST_LOG_TRIVIAL(debug) << "previous session found, trying resuming";
            resume(bot, error_write);
            bot.start_resuming();
        }

        else
        {
            BOOST_LOG_TRIVIAL(debug) << "previous session not found, trying new session";
            identify(bot, error_write);
        }

        if (error_write)
        {
            BOOST_LOG_TRIVIAL(error) << "failed to start bot session: " << error_write.message();
            bot.stop();
        }
        break;

    case opcode_type::hello_ack:
        BOOST_LOG_TRIVIAL(debug) << "received event hello ack(11)";
        break;

    case opcode_type::invalid_session:
        BOOST_LOG_TRIVIAL(debug) << "failed to resume session, starting new session";
        bot.stop_resuming();
        identify(bot, error_write);
        if (error_write)
        {
            BOOST_LOG_TRIVIAL(error) << "failed to start bot session: " << error_write.message();
            bot.stop();
        }
        break;

    default:
        BOOST_LOG_TRIVIAL(debug) << "received unhandled event(" << static_cast< uint32_t >(opcode) << ")";
        break;
    }

    if (bot.is_running())
        bot.async_listen_event();
}

} // namespace

namespace qyzk::ohno
{

bot::bot(io_context& context_io, ohno::config& config)
    : m_config(config)
    , m_context_io(context_io)
    , m_context_ssl(ssl::context::tlsv12_client)
    , m_gateway(config)
    , m_websocket(m_context_io, m_context_ssl)
    , m_heartbeat_interval(0)
    , m_buffer()
    , m_timer_heartbeat(m_context_io)
    , m_is_running(false)
    , m_is_resuming(false)
{
}

auto bot::connect(void) -> void
{
    auto const& url = m_gateway.get_url();
    auto const pos = url.find("://") + 3;
    auto const hostname = url.substr(pos);

    error_code error;
    ip::tcp::resolver resolver(m_context_io);
    auto hosts = resolver.resolve(hostname, "https", error);
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to resolve Discord gateway: " << error.message();
        return;
    }

    get_lowest_layer(m_websocket).connect(hosts, error);
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to connect to Discord gateway: " << error.message();
        return;
    }

    m_websocket.next_layer().handshake(ssl::stream_base::client, error);
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to SSL handshake with Discord gateway: " << error.message();
        return;
    }

    m_websocket.handshake(hostname, "/?v=6&encoding=json", error);
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to WebSocket handshake with Discord gateway: " << error.message();
    }

    m_is_running = true;

    BOOST_LOG_TRIVIAL(debug) << "connected to Discord gateway";
}

auto bot::disconnect(void) -> void
{
    BOOST_LOG_TRIVIAL(debug) << "disconnecting from Discord gateway";

    error_code error;
    m_websocket.next_layer().shutdown(error);
    if (error && ssl::error::stream_truncated)
    {
        BOOST_LOG_TRIVIAL(debug) << "server has closed connection";
    }
    else if (error)
    {
        BOOST_LOG_TRIVIAL(warning) << "connection to gateway has been closed ungratefully";
    }

    get_lowest_layer(m_websocket).socket().close(error);
    BOOST_LOG_TRIVIAL(debug) << "disconnected from Discord gateway";

    m_heartbeat_interval = 0;
}

auto bot::async_start_heartbeat(boost::system::error_code const& error) -> void
{
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "timer error: " << error.message();
        BOOST_LOG_TRIVIAL(error) << "stopped heartbeating";
    }

    else
    {
        beat();
        m_timer_heartbeat.expires_at(
            m_timer_heartbeat.expiry() + chrono::milliseconds(m_heartbeat_interval));
        m_timer_heartbeat.async_wait(
            boost::bind(
                &bot::async_start_heartbeat,
                this,
                boost::asio::placeholders::error));
    }
}

auto bot::beat(void) -> void
{
    auto const sequence = m_config.get_event_sequence();

    nlohmann::json payload;
    payload["op"] = 1;
    if (sequence == 0)
        payload["d"] = nullptr;
    else
        payload["d"] = sequence;
    
    error_code error;
    m_websocket.write(boost::asio::buffer(payload.dump()), error);
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to send heartbeat ping: " << error.message();
        stop();
    }

    BOOST_LOG_TRIVIAL(debug) << "sent heartbeating";
}

auto bot::async_listen_event(void) -> void
{
    m_websocket.async_read(
        m_buffer,
        boost::bind(
            handle_event,
            std::ref(*this),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

auto bot::get_heartbeat_timer() noexcept -> boost::asio::steady_timer&
{
    return m_timer_heartbeat;
}

auto bot::get_heartbeat_interval() const noexcept -> uint32_t
{
    return m_heartbeat_interval;
}

auto bot::set_heartbeat_interval(uint32_t const interval) noexcept -> void
{
    m_heartbeat_interval = interval;
}

auto bot::is_running() const noexcept -> bool
{
    return m_is_running;
}

auto bot::stop(void) noexcept -> void
{
    if (!m_is_running)
        return;

    error_code error;
    m_timer_heartbeat.cancel(error);
    if (error)
        BOOST_LOG_TRIVIAL(error) << "failed to cancel heartbeat timer";

    m_websocket.async_close(websocket::close_code::none, handle_websocket_close);

    m_is_running = false;
}

auto bot::get_websocket_buffer() noexcept -> boost::beast::flat_buffer&
{
    return m_buffer;
}

auto bot::get_websocket(void) noexcept -> bot::websocket_stream_type&
{
    return m_websocket;
}

auto bot::get_config(void) noexcept -> ohno::config&
{
    return const_cast< ohno::config& >((const_cast< ohno::bot const& >(*this).get_config()));
}

auto bot::get_config(void) const noexcept -> ohno::config const&
{
    return m_config;
}

auto bot::is_resuming(void) const noexcept -> bool
{
    return m_is_resuming;
}

auto bot::start_resuming(void) noexcept -> void
{
    m_is_resuming = true;
}

auto bot::stop_resuming(void) noexcept -> void
{
    m_is_resuming = false;;
}

} // namespace qyzk::ohno
