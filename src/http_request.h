#ifndef __QYZK_OHNO_HTTP_REQUEST_H__
#define __QYZK_OHNO_HTTP_REQUEST_H__

#include <optional> 
#include <string>

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

#include "./config.h"

namespace qyzk::ohno
{

using hosts_type = decltype(
        std::declval< boost::asio::ip::tcp::resolver >().resolve(
            std::declval< std::string >(),
            std::declval< std::string >()));

auto get_gateway_bot(
    ohno::config const& config,
    std::optional< hosts_type > const& hosts_resolved = std::optional< hosts_type >())
    -> nlohmann::json;

} // namespace qyzk::ohno

#endif
