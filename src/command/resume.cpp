#include "../command.h"
#include "../opcode.h"

namespace qyzk::ohno
{

using json = nlohmann::json;

auto resume(
    ws_stream_type& stream,
    std::string const token,
    std::string const id_session,
    uint32_t const sequence)
    -> void
{
    json data;
    data["token"] = token;
    data["session_id"] = id_session;
    data["seq"] = sequence;

    json payload;
    payload["op"] = static_cast< uint32_t >(opcode_type::resume);
    payload["d"] = data;

    stream.write(boost::asio::buffer(payload.dump()));
}

} // namespace qyzk::ohno
