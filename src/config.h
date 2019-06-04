#ifndef __QYZK_OHNO_CONFIG_H__
#define __QYZK_OHNO_CONFIG_H__

#include <filesystem>
#include <string>

#include <nlohmann/json.hpp>

#include "./cache.hpp"

namespace qyzk::ohno
{

class config_cache_descriptor
{
public:
    enum class key_type : std::size_t
    {
        session_id = 0,
        last_event_sequence = 1,
    };

    static constexpr auto number() noexcept -> std::size_t
    {
        return 2;
    }
}; // class config_cache_descriptor

class config
{
public:
    using cache_type = qyzk::cache< config_cache_descriptor, std::string, uint32_t >;

    config(nlohmann::json const& config);

    auto get_discord_hostname(void) const noexcept -> std::string const&;
    auto get_gateway_option(void) const noexcept -> std::string const&;
    auto get_gateway_version(void) const noexcept -> uint32_t;
    auto get_http_api_location(void) const noexcept -> std::string const&;
    auto get_http_api_version(void) const noexcept -> uint32_t;
    auto get_token(void) const noexcept -> std::string const&;

    auto get_cache() const noexcept -> cache_type const&;
    auto get_cache() noexcept -> cache_type&;

private:
    std::string const m_hostname_discord;
    uint32_t const m_version_api_http;
    uint32_t const m_version_gateway;
    std::string const m_location_api_http;
    std::string const m_option_gateway;
    std::string const m_token;
    cache_type m_cache;
}; // class qyzk::ohno::config

auto load_config(
    std::filesystem::path const& path_config)
    -> ohno::config;

auto save_config(
    std::filesystem::path const& path_config,
    ohno::config const& config)
    -> void;

} // namespace qyzk

#endif
