#ifndef __QYZK_OHNO_HTTP_REQUEST_H__
#define __QYZK_OHNO_HTTP_REQUEST_H__

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

auto resolve(std::string const& hostname, std::string const& service) -> hosts_type;

auto get_gateway_bot(
    ohno::config const& config,
    hosts_type const& hosts_resolved)
    -> get_gateway_bot_result;

auto connect_to_gateway(
    boost::asio::io_context& context_io,
    boost::asio::ssl::context& context_ssl,
    std::string const& url,
    std::string const& option)
    -> ws_stream_type;

auto disconnect_from_gateway(stream_type& stream) -> void;

auto send_message(
    ohno::config const& config,
    hosts_type const& hosts,
    std::string const channel,
    std::string const message)
    -> void;

auto kick(
    ohno::config const& config,
    hosts_type const& hosts,
    std::string const guild,
    std::string const id)
    -> void;

auto delete_message(
    ohno::config const& config,
    hosts_type const& hosts,
    std::string const channel,
    std::string const id)
    -> void;

} // namespace qyzk::ohno

#endif
