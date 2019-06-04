#include "../command.h"
#include "../opcode.h"

namespace qyzk::ohno
{

using json = nlohmann::json;

auto identify(ws_stream_type& stream, std::string token) -> void
{
    json properties;
    properties["$os"] = "Archlinux";
    properties["$browser"] = "oh no bot";
    properties["$device"] = "oh no bot";

    json presence;
    presence["status"] = "online";
    presence["afk"] = "false";

    json data;
    data["token"] = token;
    data["properties"] = properties;
    data["presence"] = presence;

    json payload;
    payload["d"] = data;
    payload["op"] = static_cast< uint32_t >(opcode_type::identify);

    stream.write(boost::asio::buffer(payload.dump()));
}

} // namespace qyzk::ohno
