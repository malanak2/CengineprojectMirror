cmake_minimum_required(VERSION 4.2)
project(cengineengine)
include(FetchContent)
set(CMAKE_CXX_STANDARD 26)

find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(nlohmann_json REQUIRED)

FetchContent_Declare(inipp GIT_REPOSITORY https://github.com/Rookfighter/inifile-cpp.git)
FetchContent_MakeAvailable(inipp)
FetchContent_Declare(
  cpptrace
  GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace.git
  GIT_TAG        v1.0.4 # <HASH or TAG>
)
FetchContent_MakeAvailable(cpptrace)
add_library(cengineengine SHARED Engine.hpp Engine.cpp
        Graphics/Graphics.cpp
        Graphics/Graphics.hpp
        Graphics/Program.cpp
        Graphics/Program.hpp
        Graphics/Shader.cpp
        Graphics/Shader.hpp
        Graphics/IRenderable.hpp
        Graphics/Material.cpp
        Graphics/Material.hpp
        Graphics/MaterialJson.hpp
        Config/Config.cpp
        Config/Config.hpp
        glad/src/glad.c
        glad/include/KHR/khrplatform.h
        glad/include/glad/glad.h
      )

target_include_directories(cengineengine PUBLIC glad/include)
target_include_directories(cengineengine PUBLIC ..)

set_target_properties(cengineengine PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_link_libraries(cengineengine PUBLIC spdlog::spdlog)
target_link_libraries(cengineengine PRIVATE cpptrace::cpptrace glfw inicpp::inicpp cengineutil OpenGL::GL nlohmann_json::nlohmann_json)
if(WIN32)
      add_custom_command(
              TARGET cengineengine POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E copy_if_different
              $<TARGET_FILE:cpptrace::cpptrace>
              $<TARGET_FILE_DIR:cengineengine>
      )
endif()