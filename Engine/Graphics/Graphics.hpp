//
// Created by malan on 20.04.2026.
//

#ifndef CENGINEPROJECT_GRAPHICS_HPP
#define CENGINEPROJECT_GRAPHICS_HPP
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Graphics {
public:
    int Init();
    int Tick();
    void Terminate();
private:
    GLFWwindow* window;
};

enum ShaderType {
    Vertex,
    Fragment,
};


#endif //CENGINEPROJECT_GRAPHICS_HPP
