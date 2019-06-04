#include "./opcode.h"

namespace qyzk::ohno
{

opcode_names_type opcode_names {
    { opcode_type::dispatch, "dispatch" },
    { opcode_type::heartbeat, "heartbeat" },
    { opcode_type::identify, "identify" },
    { opcode_type::status_update, "status_update" },
    { opcode_type::voice_state_update, "voice_state_update" },
    { opcode_type::unknown, "unknown" },
    { opcode_type::resume, "resume" },
    { opcode_type::reconnect, "reconnect" },
    { opcode_type::request_guild_members, "request_guild_members" },
    { opcode_type::invalid_session, "invalid_session" },
    { opcode_type::hello, "hello" },
    { opcode_type::heartbeat_ack, "heartbeat_ack" },
};

}
