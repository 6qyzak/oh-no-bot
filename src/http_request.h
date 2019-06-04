#ifndef __QYZK_OHNO_HTTP_REQUEST_H__
#define __QYZK_OHNO_HTTP_REQUEST_H__

#include <optional> 
#include <string>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <nlohmann/json.hpp>

#include "./config.h"

namespace qyzk::ohno
{

struct get_gateway_bot_result
{
    std::string url;
    uint32_t shards;
    struct {
        uint32_t total;
        uint32_t remaining;
        uint32_t reset_after;
    } session_start_limit ;
};

using hosts_type = decltype(
        std::declval< boost::asio::ip::tcp::resolver >().resolve(
            std::declval< std::string >(),
            std::declval< std::string >()));

using stream_type = boost::beast::ssl_stream< boost::beast::tcp_stream >;
using ws_stream_type = boost::beast::websocket::stream< stream_type >;

auto get_gateway_bot(
    ohno::config const& config,
    std::optional< hosts_type > const& hosts_resolved = std::optional< hosts_type >())
    -> get_gateway_bot_result;

auto connect_to_gateway(
    boost::asio::io_context& context_io,
    boost::asio::ssl::context& context_ssl,
    std::string const& url,
    std::string const& option)
    -> ws_stream_type;

auto disconnect_from_gateway(stream_type& stream) -> void;

} // namespace qyzk::ohno

#endif
