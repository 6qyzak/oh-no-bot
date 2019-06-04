#ifndef __QYZK_OHNO_EVENT_H__
#define __QYZK_OHNO_EVENT_H__

#include <cstdint>
#include <map>
#include <string>

namespace qyzk::ohno
{

enum class event_type : uint32_t
{
    hello = 0,
    ready = 1,
    resumed = 2,
    invalid_session = 3,
    channel_create = 4,
    channel_update = 5,
    channel_delete = 6,
    channel_pins_update = 7,
    guild_create = 8,
    guild_update = 9,
    guild_delete = 10,
    guild_ban_add = 11,
    guild_ban_remove = 12,
    guild_emojis_update = 13,
    guild_integrations_update = 14,
    guild_member_add = 15,
    guild_member_remove = 16,
    guild_member_update = 17,
    guild_members_chunk = 18,
    guild_role_create = 19,
    guild_role_update = 20,
    guild_role_delete = 21,
    message_create = 22,
    message_update = 23,
    message_delete = 24,
    message_delete_bulk = 25,
    message_reaction_add = 26,
    message_reaction_remove = 27,
    message_reaction_remove_all = 28,
    presence_update = 29,
    typing_start = 30,
    user_update = 31,
    voice_state_update = 32,
    voice_server_update = 33,
    webhooks_update = 34,
}; // enum class event_type

using events_type = std::map< std::string, event_type >;
extern events_type events;

} // namespace qyzk::ohno

#endif
