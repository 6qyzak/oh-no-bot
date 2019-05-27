#include <iostream>
#include <memory>

#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <nlohmann/json.hpp>

#include "./config.h"
#include "./gateway.h"

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
        trivial::severity >= trivial::debug
    );
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

    BOOST_LOG_TRIVIAL(debug) << "loading config";
    qyzk::ohno::config config;
    try
    {
        config.load(argv[1]);
    }

    catch (std::exception const& error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to load config file";
        BOOST_LOG_TRIVIAL(error) << error.what();
        return EXIT_FAILURE;
    }

    BOOST_LOG_TRIVIAL(debug) << "loaded config";
    BOOST_LOG_TRIVIAL(debug) << "using Discord API version " << config.get_api_version();
    BOOST_LOG_TRIVIAL(debug) << "using Discord Gateway version " << config.get_gateway_version();

    BOOST_LOG_TRIVIAL(debug) << "getting gateway URL";
    std::unique_ptr< qyzk::ohno::gateway > gateway_p;
    try
    {
        gateway_p.reset(new qyzk::ohno::gateway(config));
    }
    catch (std::exception const& error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to get gateway bot: " << error.what();
        return EXIT_FAILURE;
    }

    auto& gateway = *gateway_p;
    BOOST_LOG_TRIVIAL(debug) << "got gateway bot";
    BOOST_LOG_TRIVIAL(debug) << "gateway URL: " << gateway.get_url();
    BOOST_LOG_TRIVIAL(debug) << "recommended shard: " << gateway.get_shard();
    BOOST_LOG_TRIVIAL(debug) << "using shard: 1";
    BOOST_LOG_TRIVIAL(debug) << "bot session limit: " << gateway.get_session_limit_total();
    BOOST_LOG_TRIVIAL(debug) << "remaining bot session: " << gateway.get_session_limit_remaining();
    BOOST_LOG_TRIVIAL(debug) << "time before bot session resets: " << gateway.get_session_limit_resets_after();

    return EXIT_SUCCESS;
}
