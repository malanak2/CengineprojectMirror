cmake_minimum_required(VERSION 4.2)
project(cengineproject)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE Debug)
include(FetchContent)

find_package(spdlog CONFIG REQUIRED)

add_executable(cengineproject main.cpp )
add_compile_definitions(DEBUG=1 )
add_subdirectory(Engine)
add_subdirectory(Util)

target_link_libraries(cengineproject PRIVATE cengineengine cengineutil spdlog::spdlog)