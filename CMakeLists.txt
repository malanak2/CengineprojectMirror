cmake_minimum_required(VERSION 4.2)
if (WIN32)
  add_compile_options("-fmacro-prefix-map=${CMAKE_SOURCE_DIR}/=")
  set(CMAKE_TOOLCHAIN_FILE "C:\\Users\\malan\\Documents\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake" CACHE STRING "")
  set(VCPKG_TARGET_TRIPLET "x64-mingw-dynamic")
  set(VCPKG_APPLOCAL_DEPS ON)

  include(C:\\Users\\malan\\Documents\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake)

  list(APPEND CMAKE_IGNORE_PATH "C:/msys64")
endif ()
project(cengineeditor)

include(FetchContent)

set(CMAKE_GENERATOR Ninja)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE Debug)
set(IMGUI true)
set(EDITOR true)
set(PROFILE true)
if (PROFILE)
  set(TRACY_ENABLE ON CACHE BOOL "" FORCE)
  set(TRACY_ON_DEMAND ON CACHE BOOL "" FORCE) # Only captures when GUI is connected
  set(TRACY_CALLSTACK 16)
  if (!WIN32)
    set(TRACY_LIBUNWIND_BACKTRACE ON)
  endif ()
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  FetchContent_Declare(
          tracy GIT_REPOSITORY https://github.com/wolfpld/tracy.git
          GIT_TAG master
          GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(tracy)
endif ()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)

find_package(spdlog CONFIG REQUIRED)

find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)

## Reload by gemini
# 1. Glob all headers in your source directories (avoids scanning build/out folders)
file(GLOB_RECURSE ALL_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Editor/*.hpp"
)

set(UMBRELLA_INCLUDES "// AUTO-GENERATED FILE\n#pragma once\n")

# 2. Scan each file for the annotation
foreach(HEADER ${ALL_HEADERS})
  file(READ "${HEADER}" FILE_CONTENT)

  # Fast string search for the different variations
  string(FIND "${FILE_CONTENT}" "[[=Attr::Script" HAS_SHORT)
  string(FIND "${FILE_CONTENT}" "[[=Engine::Attr::Script" HAS_LONG)

  # If ANY of the variations are found (meaning the result is NOT -1)
  if(NOT HAS_SHORT EQUAL -1 OR NOT HAS_LONG EQUAL -1)
    string(APPEND UMBRELLA_INCLUDES "#include \"${HEADER}\"\n")
  endif()
endforeach()
# 3. Write to a temporary file first
set(TEMP_HEADER "${CMAKE_CURRENT_BINARY_DIR}/Generated/AllComponents.tmp.hpp")
set(UMBRELLA_HEADER "${CMAKE_CURRENT_BINARY_DIR}/Generated/AllComponents.hpp")
file(WRITE "${TEMP_HEADER}" "${UMBRELLA_INCLUDES}")

configure_file("${TEMP_HEADER}" "${UMBRELLA_HEADER}" COPYONLY)


add_executable(cengineeditor main.cpp Editor/ImGuiRenderers.cpp Editor/ImGuiRenderers.hpp Editor/ImGuiMacros.hpp Editor/ImGuiRegistrar.hpp Editor/Components/TestInputComponent.hpp Editor/Components/TestInputComponent.cpp Editor/ImGui/Window.hpp Editor/ImGui/WindowScene.hpp Editor/Components/ComponentControllable.cpp Editor/Components/ComponentTestTrigger.cpp
        Editor/ImGui/WindowObjectInspector.hpp
        Editor/ImGui/WindowChangeKeybind.hpp
        Editor/ImGui/WindowGraphicsDebug.hpp
        Editor/ImGui/WindowInputMaps.hpp
        Editor/ImGui/WindowPhysicsSystemDebug.hpp
        Editor/ImGui/WindowToggles.hpp
        Editor/Components/ComponentTestTrigger.hpp
        Editor/Components/ComponentControllable.hpp)
add_compile_definitions(DEBUG=1)

target_compile_definitions(cengineeditor PUBLIC SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
if (IMGUI)
  add_compile_definitions(IMGUI=1)
endif ()
if (EDITOR)
  add_compile_definitions(EDITOR=1)
endif ()


if (WIN32)
  get_target_property(IMGUI_INCLUDE_DIR imgui::imgui INTERFACE_INCLUDE_DIRECTORIES)

  target_compile_options(cengineeditor PUBLIC -freflection)
  target_link_libraries(cengineeditor PUBLIC stdc++exp)
  target_sources(cengineeditor PRIVATE
          ${IMGUI_INCLUDE_DIR}/imgui_impl_glfw.h
          ${IMGUI_INCLUDE_DIR}/imgui_impl_opengl3.h
  )
endif ()

target_include_directories(cengineeditor PUBLIC Editor)
add_subdirectory(Engine)
target_link_libraries(cengineeditor PRIVATE cengine spdlog::spdlog imgui::imgui implot::implot)
if (PROFILE)
  target_link_libraries(cengineeditor PRIVATE Tracy::TracyClient)
endif ()
