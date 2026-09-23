cmake_minimum_required(VERSION 4.2)
project(cengine)
include(FetchContent)
set(CMAKE_CXX_STANDARD 26)

find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp REQUIRED)

FetchContent_Declare(inipp GIT_REPOSITORY https://github.com/Rookfighter/inifile-cpp.git)
FetchContent_MakeAvailable(inipp)

add_library(cengine SHARED Engine.hpp Engine.cpp
        Object.cpp
        Object.hpp
        Interfaces/IJson.hpp
        Interfaces/IComponent.cpp
        Interfaces/IComponent.hpp
        Graphics/Components/CameraComponent.cpp
        Graphics/Components/CameraComponent.hpp
        Graphics/Graphics.cpp
        Graphics/Graphics.hpp
        Graphics/Program.cpp
        Graphics/Program.hpp
        Graphics/Shader.cpp
        Graphics/Shader.hpp
        Graphics/Material.cpp
        Graphics/Material.hpp
        Graphics/Texture.cpp
        Graphics/Texture.hpp
        Graphics/Components/ComponentRenderable.hpp
        Graphics/Components/ComponentRenderable.cpp
        Graphics/Components/ComponentAnimator.cpp
        Graphics/Components/ComponentAnimator.hpp
        Graphics/Components/ComponentSocket.cpp
        Graphics/Components/ComponentSocket.hpp
        Components/InvalidComponent.cpp
        Components/InvalidComponent.cpp
        Config/Config.cpp
        Config/Config.hpp
        External/glad/src/glad.c
        External/glad/include/KHR/khrplatform.h
        External/glad/include/glad/glad.h
        Components/TestComponent.cpp
        Components/TestComponent.hpp
        Components/RegisterComponents.hpp
        Components/RegisterComponents.cpp
        Scene.cpp
        Scene.hpp
        Graphics/Interfaces/IUniform.hpp
        Graphics/Interfaces/IUniform.cpp
        Graphics/Uniforms/UniformFloatVector.cpp
        Graphics/Uniforms/UniformFloatVector.hpp
        ScriptSystem.hpp
        ScriptSystem.cpp
        Interfaces/IIdentifiable.hpp
        Graphics/Model.hpp
        Graphics/Model.cpp
)

target_include_directories(cengine PUBLIC External/glad/include)
target_include_directories(cengine PUBLIC .)
add_subdirectory(Util)

set_target_properties(cengine PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_link_libraries(cengine PUBLIC cengineutil spdlog::spdlog )
target_link_libraries(cengine PRIVATE  glfw inicpp::inicpp cengineutil OpenGL::GL nlohmann_json::nlohmann_json glm::glm stdc++exp assimp::assimp)
if (PROFILE)
  target_link_libraries(cengine PUBLIC Tracy::TracyClient)
endif()
