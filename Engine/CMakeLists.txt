cmake_minimum_required(VERSION 4.2)
project(cengineengine)
include(FetchContent)
set(CMAKE_CXX_STANDARD 26)

find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)

FetchContent_Declare(inipp GIT_REPOSITORY https://github.com/Rookfighter/inifile-cpp.git)
FetchContent_MakeAvailable(inipp)

add_library(cengineengine SHARED Engine.hpp Engine.cpp
        Graphics/Graphics.hpp
        Graphics/Graphics.cpp
        Graphics/Program.cpp
        Graphics/Program.cpp
        Graphics/Shader.cpp
        Graphics/Shader.cpp
        Graphics/IRenderable.hpp
        Graphics/Material.cpp
        Graphics/Material.hpp
        Config/Config.cpp
        Config/Config.hpp
        glad/src/glad.c
        glad/include/KHR/khrplatform.h
        glad/include/glad/glad.h
      )

target_include_directories(cengineengine PUBLIC glad/include)

set_target_properties(cengineengine PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_link_libraries(cengineengine PUBLIC spdlog::spdlog)
target_link_libraries(cengineengine PRIVATE glfw inicpp::inicpp cengineutil OpenGL::GL)
