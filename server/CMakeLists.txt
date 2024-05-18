cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Werror -Wpedantic")

project(FtpServer)

file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_executable(FtpServer
    ${SOURCES}
    )
