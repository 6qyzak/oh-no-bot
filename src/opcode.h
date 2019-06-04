#ifndef __QYZK_OHNO_OPCODE_H__
#define __QYZK_OHNO_OPCODE_H__

#include <cstdint>
#include <map>
#include <string>

namespace qyzk::ohno
{

enum class opcode_type : uint32_t
{
    dispatch = 0,
    heartbeat = 1,
    identify = 2,
    status_update = 3,
    voice_state_update = 4,
    unknown = 5,
    resume = 6,
    reconnect = 7,
    request_guild_members = 8,
    invalid_session = 9,
    hello = 10,
    heartbeat_ack = 11,
}; // enum class opcode_type

using opcode_names_type = std::map< opcode_type, std::string >;
extern opcode_names_type opcode_names;

} // namespace qyzk::ohno

#endif
