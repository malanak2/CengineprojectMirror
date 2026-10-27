cmake_minimum_required(VERSION 4.2)
if(WIN32)
add_compile_options("-fmacro-prefix-map=${CMAKE_SOURCE_DIR}/=")
set(CMAKE_TOOLCHAIN_FILE "C:\\Users\\malan\\Documents\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake" CACHE STRING "")
set(VCPKG_TARGET_TRIPLET "x64-mingw-dynamic")
set(VCPKG_APPLOCAL_DEPS ON)

include(C:\\Users\\malan\\Documents\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake)

    list(APPEND CMAKE_IGNORE_PATH "C:/msys64")
endif()
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
if(PROFILE)
      set(TRACY_ENABLE ON CACHE BOOL "" FORCE)
      set(TRACY_ON_DEMAND ON CACHE BOOL "" FORCE) # Only captures when GUI is connected
      set(TRACY_CALLSTACK 16)
      if(!WIN32)
      set(TRACY_LIBUNWIND_BACKTRACE ON)
      endif()
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
add_executable(cengineeditor main.cpp Editor/ImGuiRenderers.cpp Editor/ImGuiRenderers.hpp Editor/ImGuiMacros.hpp Editor/ImGuiRegistrar.hpp Editor/Components/Registrar.hpp Editor/Components/Registrar.cpp Editor/Components/TestInputComponent.hpp Editor/Components/TestInputComponent.cpp Editor/ImGui/Window.hpp Editor/ImGui/WindowScene.hpp Editor/Components/ComponentControllable.cpp Editor/Components/ComponentTestTrigger.cpp)
add_compile_definitions(DEBUG=1 )
if(IMGUI)
  add_compile_definitions(IMGUI=1 )
endif()
if(EDITOR)
  add_compile_definitions(EDITOR=1 )
endif()


if (WIN32)
get_target_property(IMGUI_INCLUDE_DIR imgui::imgui INTERFACE_INCLUDE_DIRECTORIES)

target_compile_options(cengineeditor PUBLIC -freflection)
target_link_libraries(cengineeditor PUBLIC stdc++exp)
target_sources(cengineeditor PRIVATE
    ${IMGUI_INCLUDE_DIR}/imgui_impl_glfw.h
    ${IMGUI_INCLUDE_DIR}/imgui_impl_opengl3.h
)
endif()

target_include_directories(cengineeditor PUBLIC Editor)
add_subdirectory(Engine)
target_link_libraries(cengineeditor PRIVATE cengine spdlog::spdlog imgui::imgui implot::implot)
if (PROFILE)
target_link_libraries(cengineeditor PRIVATE Tracy::TracyClient)
endif()
