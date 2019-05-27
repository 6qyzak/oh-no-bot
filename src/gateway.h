#ifndef __QYZK_OHNO_GATEWAY_H__
#define __QYZK_OHNO_GATEWAY_H__

#include "./config.h"

namespace qyzk::ohno
{

class gateway
{
public:
    gateway(ohno::config const& config);
    auto get_url() const noexcept -> std::string const&;
    auto get_shard() const noexcept -> uint32_t;
    auto get_session_limit_total() const noexcept -> uint32_t;
    auto get_session_limit_remaining() const noexcept -> uint32_t;
    auto get_session_limit_resets_after() const noexcept -> uint32_t;

private:
    std::string m_url;
    uint32_t m_shard;
    uint32_t m_session_limit_total;
    uint32_t m_session_limit_remaining;
    uint32_t m_session_limit_resets_after;
};

} // namespace qyzk::ohno

#endif
