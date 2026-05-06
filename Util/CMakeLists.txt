cmake_minimum_required(VERSION 4.2)
project(cengineutil)

find_package(spdlog CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 26)

add_library(cengineutil SHARED FileUtil.cpp FileUtil.hpp)

target_link_libraries(cengineutil PRIVATE spdlog::spdlog)
