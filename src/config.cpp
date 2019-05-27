#include <exception>
#include <fstream>

#include <nlohmann/json.hpp>

#include "./config.h"

namespace qyzk::ohno
{

class config_file_error : std::exception
{
public:
    virtual auto what() const noexcept -> const char* override
    {
        return "failed to open config file";
    }
}; // class qyzk::ohno::config_file_not_found_error

config::config()
    : m_token()
    , m_version_api(0)
    , m_version_gateway(0)
    , m_url_discord("https://discordapp.com")
    , m_hostname_discord("discordapp.com")
{ }

auto config::load(std::filesystem::path const& config_path) -> void
{
    std::fstream config_file(config_path);
    if (!config_file)
        throw config_file_error();

    nlohmann::json config_json;
    config_file >> config_json;
    m_token = config_json["token"];

    auto const& version_json = config_json["version"];
    m_version_api = version_json["api"];
    m_version_gateway = version_json["gateway"];

    config_file.close();
}

auto config::get_token() const noexcept -> std::string const&
{
    return m_token;
}

auto config::get_api_version() const noexcept -> uint32_t
{
    return m_version_api;
}

auto config::get_gateway_version() const noexcept -> uint32_t
{
    return m_version_gateway;
}

auto config::get_discord_url() const noexcept -> std::string const&
{
    return m_url_discord;
}

auto config::get_discord_hostname() const noexcept -> std::string const&
{
    return m_hostname_discord;
}

} // namespace qyzk::ohno
