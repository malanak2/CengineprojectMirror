//
// Created by malan on 20.04.2026.
//

#include "Graphics.hpp"
#include "Material.hpp"
#include "Util/LoggerUtil.hpp"
#include <memory>
#include <spdlog/spdlog.h>
#include <unordered_map>

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

namespace Engine {
namespace Graphics {
// Initialize vecotrs
std::unordered_map<std::string, std::shared_ptr<Shader>> Main::vertexShaders =
    {};
std::unordered_map<std::string, std::shared_ptr<Shader>> Main::fragmentShaders =
    {};
std::unordered_map<std::string, std::shared_ptr<Material>> Main::materials = {};
int Main::Init(Config *config) {
  auto logger = ENGINE_UTIL_LOGGER;
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window =
      glfwCreateWindow(800, 600, config->window->title.c_str(), NULL, NULL);
  if (window == NULL) {
    SPDLOG_LOGGER_ERROR(spdlog::get("console"),
                        "Failed to create GLFW window.");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    SPDLOG_LOGGER_ERROR(spdlog::get("console"), "Failed to initialize GLAD.");
    return -1;
  }
  glViewport(0, 0, 800, 600);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  CHECK_GL_ERROR();
  // TODO: Remove

  std::shared_ptr<Material> m = Material::Create("materials/basic.json");
  if (!m->usable)
    return -1;
  CHECK_GL_ERROR();
  glUseProgram(m->program->id);
  CHECK_GL_ERROR();
  float vertices[] = {
      -0.5f, -0.5f, 0.0f, // left
      0.5f,  -0.5f, 0.0f, // right
      0.0f,  0.5f,  0.0f  // top
  };
  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and
  // then configure vertex attributes(s).
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // note that this is allowed, the call to glVertexAttribPointer registered VBO
  // as the vertex attribute's bound vertex buffer object so afterwards we can
  // safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // You can unbind the VAO afterwards so other VAO calls won't accidentally
  // modify this VAO, but this rarely happens. Modifying other VAOs requires a
  // call to glBindVertexArray anyways so we generally don't unbind VAOs (nor
  // VBOs) when it's not directly necessary.
  glBindVertexArray(0);
  vao = VAO;
  this->material = m;
  CHECK_GL_ERROR();
  auto a = config->graphics->CameraRot;
  a.insert(a.begin(), 0);
  /*
    camera = std::make_unique<CameraComponent>(
        "Default camera", std::vector<std::shared_ptr<IComponent>>{},
        config->graphics->CameraPos, a);

    camera->_components[ENGINE_COMPONENT_TYPE::renderable] =
        std::make_shared<ComponentRenderable>(
            "materials/basic.json",
            std::unordered_map<std::string, std::vector<float>>{{"color", {1, 0,
    0, 1}}});
      /
    std::string camerajs = camera->ToJson().dump();
    FileUtil::SaveFile("generated/camera.json", &(camerajs));
    */
  return 0;
}

int Main::Tick() {
  if (glfwWindowShouldClose(window)) {
    SPDLOG_LOGGER_INFO(spdlog::get("console"), "GLFW Window should close.");
    return -1;
  }
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  CHECK_GL_ERROR();
  material->SetupMaterial();
  material->program->SetUniform("color", 1, 0, 0, 1);
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  CHECK_GL_ERROR();
  glfwSwapBuffers(window);
  glfwPollEvents();
  return 0;
}

void Main::Terminate() { glfwTerminate(); }
} // namespace Graphics
} // namespace Engine
