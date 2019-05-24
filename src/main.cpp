#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include "./config.h"

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
    try
    {
        qyzk::ohno::config config(argv[1]);
        BOOST_LOG_TRIVIAL(debug) << config.get_token();
    }

    catch (std::exception const& error)
    {
        BOOST_LOG_TRIVIAL(error) << "failed to load config file";
        BOOST_LOG_TRIVIAL(error) << error.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
