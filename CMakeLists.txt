cmake_minimum_required(VERSION 3.9)

# Project's name
project(CHIP-8)
set(EXECUTABLE_NAME "chip-8")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++")

include_directories("include")
link_directories("lib")

# Sources & Headers
file(GLOB_RECURSE SOURCES
    ./main.cpp
    ./sources/*.cpp
)

file(GLOB HEADERS
    ./*.hpp
)

# Executable
add_executable(${EXECUTABLE_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${EXECUTABLE_NAME} sfml-graphics sfml-window sfml-system sfml-audio)
