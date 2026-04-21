cmake_minimum_required(VERSION 4.2)
project(cengineengine)
include(FetchContent)
set(CMAKE_CXX_STANDARD 26)
set(VCPKG_TARGET_TRIPLET x64-windows)

find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)


FetchContent_Declare(inipp GIT_REPOSITORY https://github.com/Rookfighter/inifile-cpp.git)
FetchContent_MakeAvailable(inipp)

add_library(cengineengine SHARED Engine.hpp Engine.cpp
        Graphics/Graphics.hpp
        Graphics/Graphics.cpp
        Config/Config.cpp
        Config/Config.hpp
        glad/src/glad.c
        glad/include/KHR/khrplatform.h
        glad/include/glad/glad.h
      )



set_target_properties(cengineengine PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_link_libraries(cengineengine PRIVATE spdlog::spdlog glfw inicpp::inicpp)
# target_link_libraries(cengineengine PRIVATE spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)
