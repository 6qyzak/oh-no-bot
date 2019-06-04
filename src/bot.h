#ifndef __QYZK_OHNO_BOT_H__
#define __QYZK_OHNO_BOT_H__

#include "./config.h"
#include "./http_request.h"

namespace qyzk::ohno
{

enum class connection_status_type
{
    connecting,
    connected,
    resuming,
};

class bot
{
public:
    using buffer_type = boost::beast::flat_buffer;

    bot(
        std::filesystem::path const& path_config,
        boost::asio::io_context& context_io,
        boost::asio::ssl::context& context_ssl,
        ohno::config& config,
        get_gateway_bot_result const& material_bot);

    auto async_listen_event(void) -> void;
    auto stop(void) -> void;

private:
    auto handle_event(
        boost::beast::error_code const& error,
        std::size_t const bytes_written)
        -> void;
    auto async_heartbeat(void) -> void;
    auto heartbeat(
        boost::beast::error_code const& error)
        -> void;
    auto handle_invalid_session(nlohmann::json const& payload) -> void;
    auto handle_event_dispatch(nlohmann::json const& payload) -> void;

    std::filesystem::path const m_path_config;
    ohno::config& m_config;
    boost::asio::io_context& m_context_io;
    boost::asio::ssl::context& m_context_ssl;
    boost::beast::flat_buffer m_buffer_event;
    ws_stream_type m_stream_gateway;
    uint32_t m_interval_heartbeat;
    boost::asio::steady_timer m_timer_heartbeat;
    connection_status_type m_status_connection;
    bool m_is_running;
};

} // namespace qyzk::ohno

#endif
