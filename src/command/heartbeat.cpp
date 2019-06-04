#include "../command.h"
#include "../opcode.h"

namespace qyzk::ohno
{

using json = nlohmann::json;

auto heartbeat(ws_stream_type& stream, uint32_t const sequence) -> void
{
    json payload;
    payload["op"] = static_cast< uint32_t >(opcode_type::heartbeat);
    if (sequence == 0)
        payload["d"] = nullptr;
    else
        payload["d"] = sequence;
    stream.write(boost::asio::buffer(payload.dump()));
}

} // namespace qyzk::ohno
