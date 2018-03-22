cmake_minimum_required(VERSION 3.10)

project(boost-asio-art)

add_executable(${PROJECT_NAME}
    "main.cpp"
    "irc_client.cpp"
    "irc_client.hpp"
    "regex_explode.hpp")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS system regex REQUIRED)

IF(Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF(Boost_FOUND)
SET(USED_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_REGEX_LIBRARY})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${USED_LIBS} pthread)

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
            )

