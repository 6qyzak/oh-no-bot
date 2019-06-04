#ifndef __QYZK_OHNO_COMMAND_H__
#define __QYZK_OHNO_COMMAND_H__

#include "./http_request.h"

namespace qyzk::ohno
{

auto identify(
    ws_stream_type& stream,
    std::string const token)
    -> void;

auto heartbeat(
    ws_stream_type& stream,
    uint32_t const sequence)
    -> void;

auto resume(
    ws_stream_type& stream,
    std::string const token,
    std::string const id_session,
    uint32_t const sequence)
    -> void;

} // namespace qyzk::ohno
#endif
