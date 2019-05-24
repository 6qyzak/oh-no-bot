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

config::config(std::filesystem::path config_path)
    : m_token()
{
    std::fstream config_file(config_path);
    if (!config_file)
        throw config_file_error();

    // throw exceptions from nlohmann::json outside constructor
    nlohmann::json config_json;
    config_file >> config_json;
    m_token = config_json["token"];

    config_file.close();
}

auto config::get_token() const noexcept -> std::string const&
{
    return m_token;
}

} // namespace qyzk::ohno
