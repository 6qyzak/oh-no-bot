#include "./event.h"

namespace qyzk::ohno
{

events_type events {
    { "HELLO", event_type::hello },
    { "READY", event_type::ready },
    { "RESUMED", event_type::resumed },
    { "INVALID_SESSION", event_type::invalid_session },
    { "CHANNEL_CREATE", event_type::channel_create },
    { "CHANNEL_UPDATE", event_type::channel_update },
    { "CHANNEL_DELETE", event_type::channel_delete },
    { "CHANNEL_PINS_UPDATE", event_type::channel_pins_update },
    { "GUILD_CREATE", event_type::guild_create },
    { "GUILD_UPDATE", event_type::guild_update },
    { "GUILD_DELETE", event_type::guild_delete },
    { "GUILD_BAN_ADD", event_type::guild_ban_add },
    { "GUILD_BAN_REMOVE", event_type::guild_ban_remove },
    { "GUILD_EMOJIS_UPDATE", event_type::guild_emojis_update },
    { "GUILD_INTEGRATIONS_UPDATE", event_type::guild_integrations_update },
    { "GUILD_MEMBER_ADD", event_type::guild_member_add },
    { "GUILD_MEMBER_REMOVE", event_type::guild_member_remove },
    { "GUILD_MEMBER_UPDATE", event_type::guild_member_update },
    { "GUILD_MEMBERS_CHUNK", event_type::guild_members_chunk },
    { "GUILD_ROLE_CREATE", event_type::guild_role_create },
    { "GUILD_ROLE_UPDATE", event_type::guild_role_update },
    { "GUILD_ROLE_DELETE", event_type::guild_role_delete },
    { "MESSAGE_CREATE", event_type::message_create },
    { "MESSAGE_UPDATE", event_type::message_update },
    { "MESSAGE_DELETE", event_type::message_delete },
    { "MESSAGE_DELETE_BULK", event_type::message_delete_bulk },
    { "MESSAGE_REACTION_ADD", event_type::message_reaction_add },
    { "MESSAGE_REACTION_REMOVE", event_type::message_reaction_remove },
    { "MESSAGE_REACTION_REMOVE_ALL", event_type::message_reaction_remove_all },
    { "PRESENCE_UPDATE", event_type::presence_update },
    { "TYPING_START", event_type::typing_start },
    { "USER_UPDATE", event_type::user_update },
    { "VOICE_STATE_UPDATE", event_type::voice_state_update },
    { "VOICE_SERVER_UPDATE", event_type::voice_server_update },
    { "WEBHOOKS_UPDATE", event_type::webhooks_update },
};

} // namespace qyzk::ohno
