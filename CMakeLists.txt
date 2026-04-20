cmake_minimum_required(VERSION 4.2)
project(cengineproject)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(cengineproject main.cpp)

add_subdirectory(Engine)

target_link_libraries(cengineproject PRIVATE cengineengine)