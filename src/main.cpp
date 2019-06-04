#include <iostream>
#include <memory>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <nlohmann/json.hpp>

#include "./botv2.h"
#include "./cache.hpp"
#include "./config.h"
#include "./http_request.h"

using namespace boost::asio;

namespace
{

auto print_usage(std::string executable) -> void
{
    std::cout << "usage:\n";
    std::cout << "    " << executable << " config.json" << std::endl;
}

auto init_logger() -> void
{
    namespace trivial = boost::log::trivial;

    boost::log::core::get()->set_filter
    (
        trivial::severity >= trivial::trace
    );
}

auto print_config(qyzk::ohno::config const& config)
{
    using key = qyzk::ohno::config_cache_descriptor::key_type;

    BOOST_LOG_TRIVIAL(debug) << "loaded config";
    BOOST_LOG_TRIVIAL(debug) << "token: " << config.get_token();
    BOOST_LOG_TRIVIAL(debug) << "discord hostname: " << config.get_discord_hostname();
    BOOST_LOG_TRIVIAL(debug) << "gateway option: " << config.get_gateway_option();
    BOOST_LOG_TRIVIAL(debug) << "http api location: " << config.get_http_api_location();
    BOOST_LOG_TRIVIAL(debug) << "gatway version: " << config.get_gateway_version();
    BOOST_LOG_TRIVIAL(debug) << "http api version: " << config.get_http_api_version();
    BOOST_LOG_TRIVIAL(debug);

    auto const& cache = config.get_cache();
    BOOST_LOG_TRIVIAL(debug) << "loaded cache";
    BOOST_LOG_TRIVIAL(debug) << "session id: " << cache.get< key::session_id >();
    BOOST_LOG_TRIVIAL(debug) << "last event sequence: " << cache.get< key::last_event_sequence >();
}

auto handle_signal(
    boost::system::error_code const& error,
    int const signal,
    std::unique_ptr< qyzk::ohno::botv2 >& bot) -> void
{
    if (error)
    {
        BOOST_LOG_TRIVIAL(error) << "error occured during waiting signal: " << error.message();
        return;
    }

    BOOST_LOG_TRIVIAL(debug) << "signal caught, stopping bot";
    bot->stop();
}

} // namespace

auto main(
    int const argc,
    char const* const argv[])
    -> int
{
    if (argc != 2)
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    try
    {
        init_logger();
    }
    catch (std::exception const& error)
    {
        std::cerr << "failed to initialize logger: " << error.what() << std::endl;
        return EXIT_FAILURE;
    }
    BOOST_LOG_TRIVIAL(debug) << "initialized logger";

    io_context context_io;
    ssl::context context_ssl(ssl::context::tlsv12_client);

    std::unique_ptr< qyzk::ohno::config > config_p;
    {
        auto json_config = qyzk::ohno::load_config(argv[1]);
        try
        {
            config_p.reset(new qyzk::ohno::config(json_config));
        }
        catch (std::exception const& error)
        {
            BOOST_LOG_TRIVIAL(error) << "failed to load config: " << error.what();
            return EXIT_FAILURE;
        }
    }

    auto& config = *config_p;
    auto const material_bot = qyzk::ohno::get_gateway_bot(config);
    if (material_bot.session_start_limit.remaining == 0)
    {
        auto const reset_after = material_bot.session_start_limit.reset_after;
        BOOST_LOG_TRIVIAL(error) << "no session is remaining try after " << (reset_after / 1000) << " seconds";
        config_p.reset();
        return EXIT_FAILURE;
    }

    std::unique_ptr< qyzk::ohno::botv2 > bot_p;
    try
    {
        bot_p.reset(new qyzk::ohno::botv2(argv[1], context_io, context_ssl, config, material_bot));
    }
    catch (std::exception const& error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to initialize bot: " << error.what();
        return EXIT_FAILURE;
    }

    boost::asio::signal_set signals(context_io, SIGINT, SIGTERM);
    signals.async_wait(
        boost::bind(
            handle_signal,
            boost::asio::placeholders::error,
            boost::asio::placeholders::signal_number,
            std::ref(bot_p)));

    auto& bot = *bot_p;
    bot.async_listen_event();
    context_io.run();

    return EXIT_SUCCESS;
}
