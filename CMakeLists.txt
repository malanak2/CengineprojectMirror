cmake_minimum_required(VERSION 4.2)
project(cengineproject)

add_compile_options("-fmacro-prefix-map=${CMAKE_SOURCE_DIR}/=")
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE Debug)
set(IMGUI true)
set(PROFILE true)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)
FetchContent_Declare(
  cpptrace
  GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace.git
  GIT_TAG        v1.0.4 # <HASH or TAG>
)
FetchContent_MakeAvailable(cpptrace)
find_package(spdlog CONFIG REQUIRED)

add_executable(cengineproject main.cpp )
add_compile_definitions(DEBUG=1 )
if(IMGUI)
  add_compile_definitions(IMGUI=1 )
endif()
add_subdirectory(Util)
add_subdirectory(Engine)

target_link_libraries(cengineproject PRIVATE cengineengine cengineutil cpptrace::cpptrace spdlog::spdlog)
if(IMGUI)
  # Add imgui
endif()
if(WIN32)
  add_custom_command(
    TARGET cengineproject POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:cpptrace::cpptrace>
    $<TARGET_FILE_DIR:cengineproject>
  )
endif()
