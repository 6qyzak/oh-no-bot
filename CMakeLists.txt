CMAKE_MINIMUM_REQUIRED (VERSION 3.14)

PROJECT ("oh no bot")

FIND_PACKAGE (OpenSSL REQUIRED)

FIND_PACKAGE (Boost "1.70.0" REQUIRED COMPONENTS log)
ADD_DEFINITIONS (-DBOOST_ALL_DYN_LINK)

SET (JSON_BuildTests OFF CACHE INTERNAL "")
ADD_SUBDIRECTORY ("./lib/json")

ADD_EXECUTABLE (
    "oh_no_bot"
    ./src/config.cpp
    ./src/gateway.cpp
    ./src/main.cpp
)

SET_PROPERTY (
    TARGET "oh_no_bot"
    PROPERTY CXX_STANDARD 17
)

TARGET_LINK_LIBRARIES (
    "oh_no_bot"
    stdc++fs
    OpenSSL::SSL
    Boost::log
    nlohmann_json::nlohmann_json
)
