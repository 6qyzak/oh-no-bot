#include <exception>
#include <fstream>

#include <boost/log/trivial.hpp>

#include <nlohmann/json.hpp>

#include "./config.h"

namespace
{

auto get_http_api_location(uint32_t const version_api_http) -> std::string
{
    return "/api/v" + std::to_string(version_api_http);
}

auto get_gateway_option(uint32_t const version_gateway) -> std::string
{
    return "?v=" + std::to_string(version_gateway) + "&encoding=json";
}

} // namespace

namespace qyzk::ohno
{

class config_file_error : std::exception
{
public:
    virtual auto what(void) const noexcept -> const char* override
    {
        return "failed to open config file";
    }
}; // class qyzk::ohno::config_file_not_found_error

config::config(nlohmann::json const& config)
    : m_hostname_discord("discordapp.com")
    , m_version_api_http(config["version"]["http_api"])
    , m_version_gateway(config["version"]["gateway"])
    , m_location_api_http(::get_http_api_location(m_version_api_http))
    , m_option_gateway(::get_gateway_option(m_version_gateway))
    , m_token(config["token"])
    , m_cache()
{
    using key = cache_type::key_type;
    auto const& json_cache = config["cache"];
    m_cache.set< key::session_id >(json_cache["session_id"]);
    m_cache.set< key::last_event_sequence >(json_cache["last_event_sequence"]);
}

auto config::get_discord_hostname(void) const noexcept -> std::string const&
{
    return m_hostname_discord;
}

auto config::get_gateway_option(void) const noexcept -> std::string const&
{
    return m_option_gateway;
}

auto config::get_gateway_version(void) const noexcept -> uint32_t
{
    return m_version_gateway;
}

auto config::get_http_api_location(void) const noexcept -> std::string const&
{
    return m_location_api_http;
}

auto config::get_http_api_version(void) const noexcept -> uint32_t
{
    return m_version_api_http;
}

auto config::get_token(void) const noexcept -> std::string const&
{
    return m_token;
}

auto config::get_cache() const noexcept -> config::cache_type const&
{
    return m_cache;
}

auto config::get_cache() noexcept -> config::cache_type&
{
    return const_cast< config::cache_type& >(const_cast< ohno::config const& >(*this).get_cache());
}

auto load_config(std::filesystem::path const& path_config) -> ohno::config
{
    std::fstream file_config(path_config);
    if (!file_config)
        throw ohno::config_file_error();
    nlohmann::json json_config;
    file_config >> json_config;
    file_config.close();
    return ohno::config(json_config);
}

auto save_config(std::filesystem::path const& path_config, ohno::config const& config) -> void
{
    using key = config_cache_descriptor::key_type;
    auto const& cache = config.get_cache();

    nlohmann::json json_cache;

    if (cache.has< key::session_id >())
        json_cache["session_id"] = cache.get< key::session_id >();
    else
        json_cache["session_id"] = "";

    if (cache.has< key::last_event_sequence >())
        json_cache["last_event_sequence"] = cache.get< key::last_event_sequence >();
    else
        json_cache["last_event_sequence"] = 0;

    nlohmann::json version;
    version["http_api"] = config.get_http_api_version();
    version["gateway"] = config.get_gateway_version();

    nlohmann::json json_config;
    json_config["token"] = config.get_token();
    json_config["version"] = version;
    json_config["cache"] = json_cache;

    std::ofstream config_file(path_config, std::ios_base::trunc);
    if (!config_file)
        throw qyzk::ohno::config_file_error();

    config_file << json_config.dump(4);
    config_file.close();
}

} // namespace qyzk::ohno
