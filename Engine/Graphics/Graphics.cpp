//
// Created by malan on 20.04.2026.
//

#include "Graphics.hpp"
#include "../../Util/FileUtil.hpp"
#include "Material.hpp"
#include <memory>
#include <spdlog/spdlog.h>
#include <unordered_map>

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

using namespace Graphics;
// Initialize vecotrs
std::unordered_map<std::string, std::shared_ptr<Shader>> Main::vertexShaders =
    {};
std::unordered_map<std::string, std::shared_ptr<Shader>> Main::fragmentShaders =
    {};

int Main::Init(Config *config) {
  auto logger = spdlog::get("console");
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window = glfwCreateWindow(800, 600, config->Window_Title.c_str(), NULL, NULL);
  if (window == NULL) {
    spdlog::get("console")->error("Failed to create GLFW window.");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    spdlog::get("console")->error("Failed to initialize glfw.");
    return -1;
  }
  glViewport(0, 0, 800, 600);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  // Setup vectors
  // TODO: Remove
  /*
  std::string src = "generated/shaders/glsl/basic.vert.glsl";
  std::string out;
  if (FileUtil::ReadFile(src, &out) != 0) {
    logger->error("Failed to read shader file");
    return -1;
  }
  std::shared_ptr<Shader> vert =
      std::shared_ptr<Shader>(new Shader(Shader::ShaderType::Vertex, out));
  logger->info("New vert shader with id of {}, is reusable? {} valid? {}",
               vert->id, vert->_reusable, vert->isValid);
  src = "../generated/shaders/glsl/basic.frag.glsl";
  out = "";
  if (FileUtil::ReadFile(src, &out) != 0) {
    logger->error("Failed to read shader file");
    return -1;
  }
  std::shared_ptr<Shader> frag =
      std::shared_ptr<Shader>(new Shader(Shader::ShaderType::Fragment, out));
  logger->info("New frag shader with id of {}, is reusable? {} valid? {}",
               frag->id, frag->_reusable, frag->isValid);
  Program *prog = new Program({vert, frag});
  logger->info("New frag shader with id of {}, is valid? {}", prog->id,
               prog->isValid);
  if (!prog->isValid) {
    logger->error("Failed to link program");
    return -1;
  }
  */
  std::unique_ptr<Material> m =
      std::unique_ptr<Material>(new Material("materials/basic.json"));
  glUseProgram(m->program->id);
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
  program = m->program;

  return 0;
}

int Main::Tick() {
  if (glfwWindowShouldClose(window)) {
    spdlog::get("console")->info("GLFW Window should close.");
    return -1;
  }
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glUseProgram(program->id);
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glfwSwapBuffers(window);
  glfwPollEvents();
  return 0;
}

void Main::Terminate() { glfwTerminate(); }
