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
set(DEBUG true)
set(UNIT_TESTING false)
set(EDITOR true)
set(PROFILE true)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (DEBUG)
  add_compile_definitions(DEBUG=1)
endif ()
if (UNIT_TESTING)
  add_compile_definitions(UNIT_TESTING=1)
endif ()
if (EDITOR)
  add_compile_definitions(EDITOR=1)
endif ()

if (PROFILE)
  set(TRACY_ENABLE ON CACHE BOOL "" FORCE)
  set(TRACY_ON_DEMAND ON CACHE BOOL "" FORCE) # Only captures when GUI is connected
  set(TRACY_CALLSTACK 16)
  if (NOT WIN32)
    set(TRACY_LIBUNWIND_BACKTRACE ON)
  endif ()
  FetchContent_Declare(
          tracy GIT_REPOSITORY https://github.com/wolfpld/tracy.git
          GIT_TAG master
          GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(tracy)
endif ()

find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 CONFIG QUIET)
if (NOT glfw3_FOUND)
  find_package(glfw3 REQUIRED)
endif ()

FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG docking
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
add_library(imgui::imgui ALIAS imgui)

target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw)

if (DEBUG)
  target_compile_definitions(imgui PUBLIC
          IMGUI_ENABLE_TEST_ENGINE
          IMGUI_TEST_ENGINE_ENABLE_COROUTINE_STDTHREAD_IMPL=1
  )
endif ()

# --- Build ImPlot from source against our ImGui target ---
FetchContent_Declare(
        implot
        GIT_REPOSITORY https://github.com/epezent/implot.git
        GIT_TAG master
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(implot)

add_library(implot STATIC
        ${implot_SOURCE_DIR}/implot.cpp
        ${implot_SOURCE_DIR}/implot_demo.cpp
        ${implot_SOURCE_DIR}/implot_items.cpp
)
add_library(implot::implot ALIAS implot)

target_include_directories(implot PUBLIC ${implot_SOURCE_DIR})
target_link_libraries(implot PUBLIC imgui::imgui)

include(cmake/ScriptHeaders.cmake)
generate_script_header(${CMAKE_BINARY_DIR}/Generated/EditorScripts.hpp
        DIRS ${CMAKE_SOURCE_DIR}/Editor)

if (WIN32)
  add_compile_options(-freflection)
endif ()

if (DEBUG)
  set(IMGUI_TE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Editor/External/imgui_test_engine/imgui_test_engine")

  file(GLOB IMGUI_TE_SOURCES CONFIGURE_DEPENDS "${IMGUI_TE_DIR}/*.cpp")
  file(GLOB IMGUI_TE_HEADERS CONFIGURE_DEPENDS "${IMGUI_TE_DIR}/*.h")
else()
  set(IMGUI_TE_SOURCES "")
endif ()

add_executable(cengineeditor
        main.cpp
        Editor/ImGuiRenderers.cpp
        Editor/ImGuiRenderers.hpp
        Editor/ImGuiMacros.hpp
        Editor/ImGuiRegistrar.hpp
        Editor/Components/TestInputComponent.hpp
        Editor/Components/TestInputComponent.cpp
        Editor/ImGui/Window.hpp
        Editor/ImGui/WindowScene.hpp
        Editor/Components/ComponentControllable.cpp
        Editor/Components/ComponentTestTrigger.cpp
        Editor/ImGui/WindowObjectInspector.hpp
        Editor/ImGui/WindowChangeKeybind.hpp
        Editor/ImGui/WindowGraphicsDebug.hpp
        Editor/ImGui/WindowInputMaps.hpp
        Editor/ImGui/WindowPhysicsSystemDebug.hpp
        Editor/ImGui/WindowToggles.hpp
        Editor/Components/ComponentTestTrigger.hpp
        Editor/Components/ComponentControllable.hpp
        Editor/ReflectionRegistrar.hpp
        Editor/ImGui/WindowReflectionOverview.hpp
        Editor/Testing/Testing.hpp
        Editor/Testing/Testing.cpp
        ${IMGUI_TE_SOURCES}
        Editor/Testing/TestingComponent.cpp
        Editor/Testing/TestingComponent.hpp
        Editor/Components/ComponentUserScene.hpp
        Editor/Components/ComponentUserScene.cpp
        Editor/EditorScripts.cpp
)

if (DEBUG)
  target_compile_definitions(cengineeditor PUBLIC SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
  target_include_directories(cengineeditor PUBLIC ${IMGUI_TE_DIR})

  if (NOT WIN32)
    find_package(Threads REQUIRED)
    target_link_libraries(cengineeditor PRIVATE Threads::Threads)
  endif ()
endif ()

if (WIN32)
  target_link_libraries(cengineeditor PUBLIC stdc++exp)
endif ()

target_include_directories(cengineeditor PUBLIC Editor)
target_include_directories(cengineeditor PRIVATE ${CMAKE_BINARY_DIR}/Generated)
add_subdirectory(Engine)
target_compile_options(cengineeditor PUBLIC -freflection)
target_link_libraries(cengineeditor PRIVATE cengine spdlog::spdlog imgui::imgui implot::implot)
if (PROFILE)
  target_link_libraries(cengineeditor PRIVATE Tracy::TracyClient)
endif ()