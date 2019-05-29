#include <exception>

#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/log/trivial.hpp>
#include <nlohmann/json.hpp>

#include "./gateway.h"

namespace
{

class gateway_error : public std::exception
{
    virtual auto what() const noexcept -> char const* override
    {
        return "gateway error";
    }
};

auto set_sni(
    boost::beast::ssl_stream< boost::beast::tcp_stream >& stream,
    std::string const& hostname) -> bool
{
    return SSL_set_tlsext_host_name(stream.native_handle(), hostname.c_str()) == 1;
}

auto secure_request_get(
    boost::asio::io_context& context_io,
    boost::beast::ssl_stream< boost::beast::tcp_stream >& stream,
    std::string const& hostname,
    std::string const& target,
    std::string const& auth = "")
    -> nlohmann::json
{
    using namespace boost::asio;
    using namespace boost::beast;

    ip::tcp::resolver resolver(context_io);
    auto hosts = resolver.resolve(hostname, "https");

    error_code error;
    get_lowest_layer(stream).connect(hosts, error);
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to connect to host (" << hostname << "): " << error.message();
        throw gateway_error();
    }

    stream.handshake(ssl::stream_base::client, error);
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to do SSL handshake: " << error.message();
        get_lowest_layer(stream).socket().close(error);
        throw gateway_error();
    }

    http::request< http::string_body > request { http::verb::get, target, 11, };
    request.set(http::field::host, hostname);
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    if (!auth.empty())
        request.set(http::field::authorization, auth);
    BOOST_LOG_TRIVIAL(debug) << "sending HTTP " << target << " request";
    http::write(stream, request, error);
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to send HTTP request: " << error.message();
    }
    else
    {
        BOOST_LOG_TRIVIAL(debug) << "sent HTTP request";
    }

    flat_buffer buffer;
    http::response< http::string_body > response;
    BOOST_LOG_TRIVIAL(debug) << "receiving GET " << target << " response";
    http::read(stream, buffer, response, error);
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to receive HTTP response: " << error.message();
    }
    else
    {
        BOOST_LOG_TRIVIAL(debug) << "received HTTP response";
        BOOST_LOG_TRIVIAL(debug) << response.result();
    }

    stream.shutdown(error);
    if (error && error == ssl::error::stream_truncated)
    {
        BOOST_LOG_TRIVIAL(debug) << "server has closed connection";
    }
    else if (error)
    {
        BOOST_LOG_TRIVIAL(warning) << "connection has been closed ungracefully: " << error.message();
    }
    get_lowest_layer(stream).socket().close(error);

    nlohmann::json response_json;
    try
    {
        response_json = nlohmann::json::parse(response.body());
    }
    catch (std::exception const& error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to parse response body: " << error.what();
        throw gateway_error();
    }
    return response_json;
}

auto request_get_discord_bot(qyzk::ohno::config const& config) -> nlohmann::json
{
    using namespace boost::asio;
    using namespace boost::beast;

    ssl::context context_ssl(ssl::context::tlsv12_client);

    io_context context_io;
    ssl_stream< tcp_stream > stream(context_io, context_ssl);

    if (!set_sni(stream, config.get_discord_hostname()))
    {
        BOOST_LOG_TRIVIAL(error) << "failed to set SNI";
        throw gateway_error();
    }

    return secure_request_get(
        context_io,
        stream,
        config.get_discord_hostname(),
        "/api/v" + std::to_string(config.get_api_version()) + "/gateway/bot",
        "Bot " + config.get_token());
}

} // namespace

namespace qyzk::ohno
{

gateway::gateway(ohno::config const& config)
    : m_url()
    , m_shard(0)
    , m_session_limit_total(0)
    , m_session_limit_remaining(0)
    , m_session_limit_resets_after(0)
{
    BOOST_LOG_TRIVIAL(debug) << "getting gateway bot";

    auto response = request_get_discord_bot(config);
    m_url = response["url"];
    m_shard = response["shards"];

    auto session_limit = response["session_start_limit"];
    m_session_limit_total = session_limit["total"];
    m_session_limit_remaining = session_limit["remaining"];
    m_session_limit_resets_after = session_limit["reset_after"];

    BOOST_LOG_TRIVIAL(debug) << "got gateway bot";
    BOOST_LOG_TRIVIAL(debug) << "gateway URL: " << get_url();
    BOOST_LOG_TRIVIAL(debug) << "recommended shard: " << get_shard();
    BOOST_LOG_TRIVIAL(debug) << "using shard: 1";
    BOOST_LOG_TRIVIAL(debug) << "bot session limit: " << get_session_limit_total();
    BOOST_LOG_TRIVIAL(debug) << "remaining bot session: " << get_session_limit_remaining();
    BOOST_LOG_TRIVIAL(debug) << "time before bot session resets: " << get_session_limit_resets_after();

}

auto gateway::get_url() const noexcept -> std::string const&
{
    return m_url;
}

auto gateway::get_shard() const noexcept -> uint32_t
{
    return m_shard;
}

auto gateway::get_session_limit_total() const noexcept -> uint32_t
{
    return m_session_limit_total;
}

auto gateway::get_session_limit_remaining() const noexcept -> uint32_t
{
    return m_session_limit_remaining;
}

auto gateway::get_session_limit_resets_after() const noexcept -> uint32_t
{
    return m_session_limit_resets_after;
}

} // namespace qyzk::ohno
