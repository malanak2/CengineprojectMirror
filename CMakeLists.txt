cmake_minimum_required(VERSION 4.2)
project(cengineeditor)

include(FetchContent)
if(WIN32)
add_compile_options("-fmacro-prefix-map=${CMAKE_SOURCE_DIR}/=")
endif()
set(CMAKE_GENERATOR Ninja)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE Debug)
set(IMGUI true)
set(EDITOR true)
set(PROFILE true)
if(PROFILE)
      set(TRACY_ENABLE ON CACHE BOOL "" FORCE)
      set(TRACY_ON_DEMAND ON CACHE BOOL "" FORCE) # Only captures when GUI is connected
      set(TRACY_CALLSTACK 16)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
      FetchContent_Declare(
        tracy        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
      )
      FetchContent_MakeAvailable(tracy)
    endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)

find_package(spdlog CONFIG REQUIRED)

find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
add_executable(cengineeditor main.cpp Editor/ImGuiComponent.hpp Editor/ImGuiRenderers.cpp Editor/ImGuiRenderers.hpp Editor/ImGuiMacros.hpp)
add_compile_definitions(DEBUG=1 )
if(IMGUI)
  add_compile_definitions(IMGUI=1 )
endif()
if(EDITOR)
  add_compile_definitions(EDITOR=1 )
endif()
add_subdirectory(Engine)

target_link_libraries(cengineeditor PRIVATE cengine spdlog::spdlog imgui::imgui implot::implot)
if (PROFILE)
target_link_libraries(cengineeditor PRIVATE Tracy::TracyClient)
endif()
if(IMGUI)
  # Add imgui
endif()
