#ifndef __QYZK_OHNO_CONFIG_H__
#define __QYZK_OHNO_CONFIG_H__

#include <filesystem>

#include <boost/asio/ip/tcp.hpp>

namespace qyzk::ohno
{

class config
{
public:
    using hosts_type = decltype(
        std::declval< boost::asio::ip::tcp::resolver >().resolve(
            std::declval< std::string >(),
            std::declval< std::string >()));

    config();
    auto load(std::filesystem::path const& config_path) -> void;
    auto get_token() const noexcept -> std::string const&;
    auto get_api_version() const noexcept -> uint32_t;
    auto get_gateway_version() const noexcept -> uint32_t;
    auto get_discord_url() const noexcept -> std::string const&;
    auto get_discord_api_url() const noexcept -> std::string const&;
    auto get_discord_hostname() const noexcept -> std::string const&;
    auto get_discord_hosts() const noexcept -> hosts_type const&;

private:
    std::string m_token;
    uint32_t m_version_api;
    uint32_t m_version_gateway;
    std::string const m_url_discord;
    std::string m_url_discord_api;
    std::string const m_hostname_discord;
    hosts_type m_hosts_discord;
}; // class qyzk::ohno::config

} // namespace qyzk

#endif
