#ifndef __QYZK_OHNO_CONFIG_H__
#define __QYZK_OHNO_CONFIG_H__

#include <filesystem>

namespace qyzk::ohno
{

class config
{
public:
    config(std::filesystem::path config_path);
    auto get_token() const noexcept -> std::string const&;

private:
    std::string m_token;
}; // class qyzk::ohno::config

} // namespace qyzk

#endif
