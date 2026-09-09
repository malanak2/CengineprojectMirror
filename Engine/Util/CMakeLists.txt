cmake_minimum_required(VERSION 4.2)
project(cengineutil)

set(CMAKE_CXX_STANDARD 26)

find_package(spdlog CONFIG REQUIRED)

add_library(cengineutil SHARED FileUtil.cpp FileUtil.hpp LoggerUtil.hpp)
set_target_properties(cengineutil PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_link_libraries(cengineutil PUBLIC spdlog::spdlog)
