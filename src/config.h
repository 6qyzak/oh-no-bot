#ifndef __QYZK_OHNO_CONFIG_H__
#define __QYZK_OHNO_CONFIG_H__

#include <filesystem>
#include <string>

namespace qyzk::ohno
{

class config
{
public:
    config();
    auto load(std::filesystem::path const& config_path) -> void;
    auto get_token() const noexcept -> std::string const&;
    auto get_api_version() const noexcept -> uint32_t;
    auto get_gateway_version() const noexcept -> uint32_t;
    auto get_discord_url() const noexcept -> std::string const&;
    auto get_discord_hostname() const noexcept -> std::string const&;

private:
    std::string m_token;
    uint32_t m_version_api;
    uint32_t m_version_gateway;
    std::string const m_url_discord;
    std::string m_url_discord_api;
    std::string const m_hostname_discord;
}; // class qyzk::ohno::config

} // namespace qyzk

#endif
