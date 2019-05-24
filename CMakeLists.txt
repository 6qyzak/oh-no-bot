CMAKE_MINIMUM_REQUIRED (VERSION 3.14)

PROJECT ("oh no bot")

FIND_PACKAGE (Boost REQUIRED log)
ADD_DEFINITIONS (-DBOOST_ALL_DYN_LINK)

SET (JSON_BuildTests OFF CACHE INTERNAL "")
ADD_SUBDIRECTORY ("./lib/json")

ADD_EXECUTABLE (
    "oh_no_bot"
    ./src/config.cpp
    ./src/main.cpp
)

SET_PROPERTY (
    TARGET "oh_no_bot"
    PROPERTY CXX_STANDARD 17
)

TARGET_LINK_LIBRARIES (
    "oh_no_bot"
    stdc++fs
    Boost::log
    nlohmann_json::nlohmann_json
)
