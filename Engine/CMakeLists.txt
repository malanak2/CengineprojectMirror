cmake_minimum_required(VERSION 4.2)
project(cengineengine)

set(CMAKE_CXX_STANDARD 26)
set(VCPKG_TARGET_TRIPLET x64-windows)

find_package(spdlog CONFIG REQUIRED)

add_library(cengineengine SHARED Engine.hpp Engine.cpp)
set_target_properties(cengineengine PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_link_libraries(cengineengine PRIVATE spdlog::spdlog)
# target_link_libraries(cengineengine PRIVATE spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)
