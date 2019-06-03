#include <exception>
#include <type_traits>

#include <boost/log/trivial.hpp>

#include "./http_request.h"

using namespace boost::asio;
using namespace boost::beast;

namespace
{

class ssl_sni_setting_error : public std::exception
{
public:
    virtual auto what(void) const noexcept -> const char* override
    {
        return "failed to set ssl sni";
    }
};

class hostname_resolve_error : public std::exception
{
public:
    virtual auto what(void) const noexcept -> const char* override
    {
        return "failed to resolve hostname";
    }
};

class http_connection_error : public std::exception
{
public:
    virtual auto what(void) const noexcept -> const char* override
    {
        return "failed to connect to host";
    }
};

class ssl_handshake_error : public std::exception
{
public:
    virtual auto what(void) const noexcept -> const char* override
    {
        return "failed to establish secure connection";
    }
};

class http_request_send_error : public std::exception
{
public:
    virtual auto what(void) const noexcept -> const char* override
    {
        return "failed to send http request";
    }
};

class http_response_receive_error : public std::exception
{
public:
    virtual auto what(void) const noexcept -> const char* override
    {
        return "failed to receive http response";
    }
};

class http_response_parse_error : public std::exception
{
public:
    virtual auto what(void) const noexcept -> const char* override
    {
        return "failed to parse http response";
    }
};

auto set_sni(qyzk::ohno::stream_type& stream, std::string const& hostname) -> void
{
    if (SSL_set_tlsext_host_name(stream.native_handle(), hostname.c_str()) != 1)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to set ssl sni";
        throw ssl_sni_setting_error();
    }
}

} // namespace

namespace qyzk::ohno
{

auto resolve(std::string const& hostname, std::string const& service) -> hosts_type
{
    io_context context_io;
    ip::tcp::resolver resolver(context_io);
    error_code error;
    auto hosts = resolver.resolve(hostname, service, error);
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to resolve host: " << error.message();
        throw hostname_resolve_error();
    }
    return hosts;
}

auto secure_connect(
    io_context& context_io,
    ssl::context& context_ssl,
    std::string const& hostname,
    hosts_type const& hosts)
    -> stream_type
{
    stream_type stream(context_io, context_ssl);
    set_sni(stream, hostname);

    error_code error;
    get_lowest_layer(stream).connect(hosts, error);
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to connect to host: " << error.message();
        throw http_connection_error();
    }

    stream.handshake(ssl::stream_base::client, error);
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to establish secure connection: " << error.message();
        get_lowest_layer(stream).close();
        throw ssl_handshake_error();
    }

    return stream;
}

auto secure_disconnect(stream_type& stream) -> void
{
    error_code error;
    stream.shutdown(error);
    if (error)
    {
        if (error == ssl::error::stream_truncated)
            BOOST_LOG_TRIVIAL(debug) << "server has closed connection";
        else
            BOOST_LOG_TRIVIAL(warning) << "ssl connection has closed ungracefully";
    }

    get_lowest_layer(stream).close();
}

auto disconnect_from_gateway(stream_type& stream) -> void
{
    secure_disconnect(stream);
}

auto send_request(stream_type& stream, http::request< http::string_body > const& request) -> nlohmann::json
{
    error_code error;
    http::write(stream, request, error);
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to send http request: " << error.message();
        throw http_request_send_error();
    }

    flat_buffer buffer;
    http::response< http::string_body > response;

    http::read(stream, buffer, response, error);
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to receive response: " << error.message();
        throw http_response_receive_error();
    }

    BOOST_LOG_TRIVIAL(debug);
    BOOST_LOG_TRIVIAL(debug) << "received http response";
    BOOST_LOG_TRIVIAL(debug) << "result: " << response.result_int() << " " << response.result();

    nlohmann::json body;
    try
    {
        body = nlohmann::json::parse(response.body());
    }
    catch (std::exception const& error)
    {
        BOOST_LOG_TRIVIAL(warning) << "failed to parse response body: " << error.what();
        BOOST_LOG_TRIVIAL(warning) << response.body();
        throw http_response_parse_error();
    }

    BOOST_LOG_TRIVIAL(debug) << body.dump(4);
    return body;
}

auto get_gateway_bot(
    ohno::config const& config,
    std::optional< hosts_type > const& hosts_resolved)
    -> get_gateway_bot_result
{
    io_context context_io;
    ssl::context context_ssl(ssl::context::tlsv12_client);

    hosts_type hosts = hosts_resolved.value_or(
        ohno::resolve(
            config.get_discord_hostname(),
            "https"));

    auto stream = secure_connect(
        context_io,
        context_ssl,
        config.get_discord_hostname(),
        hosts);

    http::request< http::string_body > request { http::verb::get, config.get_http_api_location() + "/gateway/bot", 11, };
    request.set(http::field::host, config.get_discord_hostname());
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    request.set(http::field::authorization, "Bot " + config.get_token());

    nlohmann::json response;
    try
    {
        response = send_request(stream, request);
    }
    catch (std::exception const& error)
    {
        secure_disconnect(stream);
        throw;
    }

    secure_disconnect(stream);

    auto const& session_start_limit = response["session_start_limit"];
    get_gateway_bot_result result {
        response["url"],
        response["shards"],
        {
            session_start_limit["total"],
            session_start_limit["remaining"],
            session_start_limit["reset_after"],
        },
    };
    return result;
}

auto connect_to_gateway(
    boost::asio::io_context& context_io,
    boost::asio::ssl::context& context_ssl,
    std::string const& url,
    std::string const& option)
    -> stream_type
{
    const auto& hostname = url;

    stream_type stream(context_io, context_ssl);
    set_sni(stream, hostname);

    auto hosts = resolve(hostname, "https");
    secure_connect(context_io, context_ssl, hostname, hosts);

    return stream;
}

} // namespace qyzk::ohno
