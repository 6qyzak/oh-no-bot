#ifndef __QYZK_OHNO_CONFIG_H__
#define __QYZK_OHNO_CONFIG_H__

#include <filesystem>
#include <string>

namespace qyzk::ohno
{

class config
{
public:
    config(void);
    auto load(std::filesystem::path const& config_path) -> void;
    auto get_token(void) const noexcept -> std::string const&;
    auto get_api_version(void) const noexcept -> uint32_t;
    auto get_gateway_version(void) const noexcept -> uint32_t;
    auto get_discord_url(void) const noexcept -> std::string const&;
    auto get_discord_hostname(void) const noexcept -> std::string const&;

    auto has_session_id(void) const noexcept -> bool;
    auto set_session_id(std::string const& session_id) -> void;
    auto get_session_id(void) const noexcept -> std::string const&;

    auto has_event_sequence(void) const noexcept -> bool;
    auto set_event_sequence(uint32_t const sequence) -> void;
    auto get_event_sequence(void) const noexcept -> uint32_t;

    auto save_cache(void) -> void;

private:
    std::filesystem::path m_path_config;
    std::string m_token;
    uint32_t m_version_api;
    uint32_t m_version_gateway;
    std::string const m_url_discord;
    std::string m_url_discord_api;
    std::string const m_hostname_discord;
    std::string m_id_session;
    uint32_t m_sequence_event;
}; // class qyzk::ohno::config

} // namespace qyzk

#endif
