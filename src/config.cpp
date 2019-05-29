#include <exception>
#include <fstream>

#include <boost/log/trivial.hpp>

#include <nlohmann/json.hpp>

#include "./config.h"

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

} // namespace qyzk::ohno

namespace
{

auto load_from_file(std::filesystem::path const& path_config) -> nlohmann::json
{
    std::fstream config_file(path_config);
    if (!config_file)
        throw qyzk::ohno::config_file_error();

    nlohmann::json config_json;
    config_file >> config_json;

    config_file.close();
    return config_json;
}

} // namespace

namespace qyzk::ohno
{

config::config(void)
    : m_token()
    , m_version_api(0)
    , m_version_gateway(0)
    , m_url_discord("https://discordapp.com")
    , m_hostname_discord("discordapp.com")
    , m_id_session()
{ }

auto config::load(std::filesystem::path const& config_path) -> void
{
    m_path_config = config_path;
    auto config = load_from_file(config_path);

    m_token = config["token"];

    auto const& version_json = config["version"];
    m_version_api = version_json["api"];
    m_version_gateway = version_json["gateway"];

    m_id_session = config["cache"]["session_id"];
}

auto config::save_cache(void) -> void
{
    auto config_json = load_from_file(m_path_config);
    auto& cache_json = config_json["cache"];
    cache_json["session_id"] = m_id_session;
    cache_json["event_sequence"] = m_sequence_event;

    std::ofstream config_file(m_path_config, std::ios_base::trunc);
    if (!config_file)
        throw qyzk::ohno::config_file_error();

    config_file << config_json.dump(4);

    config_file.close();
}

auto config::get_token(void) const noexcept -> std::string const&
{
    return m_token;
}

auto config::get_api_version(void) const noexcept -> uint32_t
{
    return m_version_api;
}

auto config::get_gateway_version(void) const noexcept -> uint32_t
{
    return m_version_gateway;
}

auto config::get_discord_url(void) const noexcept -> std::string const&
{
    return m_url_discord;
}

auto config::get_discord_hostname(void) const noexcept -> std::string const&
{
    return m_hostname_discord;
}

auto config::has_session_id(void) const noexcept -> bool
{
    return !m_id_session.empty();
}

auto config::set_session_id(std::string const& session_id) -> void
{
    m_id_session = session_id;
}

auto config::get_session_id(void) const noexcept -> std::string const&
{
    return m_id_session;
}

auto config::has_event_sequence(void) const noexcept -> bool
{
    return m_sequence_event != 0;
}

auto config::set_event_sequence(uint32_t const sequence) -> void
{
    m_sequence_event = sequence;
}

auto config::get_event_sequence(void) const noexcept -> uint32_t
{
    return m_sequence_event;
}

} // namespace qyzk::ohno
