#include "Program.hpp"
#include "Graphics.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/Uniforms/UniformFloatVector.hpp"
#include "spdlog/spdlog.h"
#include <cpptrace/basic.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <vector>
using namespace Engine::Graphics;
using json = nlohmann::json;
Program::Program(std::vector<UniformJson> uniforms_json,
                 std::vector<std::shared_ptr<Shader>> shaders,
                 std::string texpath, bool uses_camera) {
  _uses_camera = uses_camera;
  auto logger = spdlog::get("console");
  unsigned int program;
  program = glCreateProgram();
  for (int i = 0; i < shaders.size(); i++) {
    std::shared_ptr<Shader> shad = shaders[i];
    shaders[i] = shad;
    if (!shad->isValid) {
      SPDLOG_LOGGER_WARN(
          logger, "Tried to add an invalid shader to a program, ignoring...");
      continue;
    }
    glAttachShader(program, shad->id);
  }
  glLinkProgram(program);
  int success;
  char infoLog[512];
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    SPDLOG_LOGGER_ERROR(logger, "ERROR::PROGRAM::LINKING_FAILED {}", infoLog);
    cpptrace::generate_trace().print();
    isValid = false;
    return;
  }
  isValid = true;
  id = program;
  for (int i = 0; i < shaders.size(); i++) {
    std::shared_ptr<Shader> shad = shaders[i];
    glDetachShader(program, shad->id);
    if (!shad->_reusable) {
      shad->Delete();
      continue;
    }
  }
  unsigned int ubo;
  glGenBuffers(1, &ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, ubo);
  unsigned int total_size = _uses_camera ? 128 : 0;
  for (auto uniform : uniforms_json) {
    UniformInfo ui;
    ui.id = ubo;
    ui.offset = (unsigned int)uniform.size;
    ui.name = std::make_shared<std::string>(uniform.name);
    uniforms[uniform.name] = std::make_shared<UniformFloatVector>(
        Vector, ubo, ui.offset, std::make_shared<std::string>(uniform.name),
        std::vector<float>());
    uniforms_info[uniform.bind_point] = uniform.name;
    total_size += ui.offset;
    CHECK_GL_ERROR();
    SPDLOG_LOGGER_INFO(spdlog::get("console"),
                       "Created buffer {} with pos {} and size {}",
                       uniform.name, uniform.bind_point, uniform.size);
  }
  if (texpath != "") {
    std::shared_ptr<Texture> tex =
        Texture::Create(texpath, Main::FallbackTexture->texture);
    glBindTexture(GL_TEXTURE_2D, tex->texture);
  }

  glBufferData(GL_UNIFORM_BUFFER, total_size, NULL, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  CHECK_GL_ERROR();
}

/*
// Link the block to binding point 0
glUniformBlockBinding(id, blockIndex, 0);
// Bind the buffer to binding point 0
glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
GLint numUniforms;
glGetActiveUniformBlockiv(id, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS,
                          &numUniforms);

std::vector<GLint> uniformIndices(numUniforms);
glGetActiveUniformBlockiv(id, blockIndex,
                          GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES,
                          uniformIndices.data());

std::vector<GLint> offsets(numUniforms);
glGetActiveUniformsiv(id, numUniforms, (GLuint *)uniformIndices.data(),
                      GL_UNIFORM_OFFSET, offsets.data());

for (int i = 0; i < numUniforms; i++) {
  char name[256];
  GLsizei length;
  glGetActiveUniformName(id, uniformIndices[i], 256, &length, name);

  std::string uniformName(name);

  // Clean up the name (remove "MaterialData." prefix if present)
  size_t dot = uniformName.find('.');
  if (dot != std::string::npos)
    uniformName = uniformName.substr(dot + 1);

  uniforms[uniformName] = {offsets[i]};
  spdlog::get("console")->info("Found Uniform: {} at offset {}", uniformName,
                               offsets[i]);
}
*/

Program::~Program() {
  glDeleteProgram(id);
  id = 0;
  for (auto [name, uniform] : uniforms) {
    glDeleteBuffers(1, &(uniform)->info.id);
  }
  uniforms.clear();
}

/// Assumes program is used
void Program::SetUniform(std::string uniform, float value, bool camera) {

  if (!uniforms.contains(uniform)) {
    SPDLOG_LOGGER_INFO(spdlog::get("console"), "Uniform not found: {}",
                       uniform);
    return;
  }
  glBindBuffer(GL_UNIFORM_BUFFER, uniforms[uniform]->info.id);
  glBufferSubData(GL_UNIFORM_BUFFER, GetUniformOffset(uniform, camera),
                  sizeof(float), &value);
  CHECK_GL_ERROR();
}

/// Assumes program is used
void Program::SetUniform(std::string uniform, float v1, float v2, float v3,
                         float v4, bool camera) {
  if (!uniforms.contains(uniform)) {
    SPDLOG_LOGGER_INFO(spdlog::get("console"), "Uniform not found: {}",
                       uniform);
    return;
  }
  glBindBuffer(GL_UNIFORM_BUFFER, uniforms[uniform]->info.id);
  float data[4] = {v1, v2, v3, v4};
  /* spdlog::get("console")->info(
       "Binding ubo {} id {}, setting the data to {} {} {} {}", uniform,
       uniforms[uniform], v1, v2, v3, v4);*/
  glBufferSubData(GL_UNIFORM_BUFFER, GetUniformOffset(uniform, camera),
                  sizeof(data), data);
  CHECK_GL_ERROR();
}

void Program::SetUniform(std::string key, std::vector<float> values,
                         bool camera) {
  switch (values.size()) {
  case 1: {
    SetUniform(key, values[0], camera);
    break;
  }
  case 4: {
    SetUniform(key, values[0], values[1], values[2], values[3], camera);
    break;
  }
  default: {
    SPDLOG_LOGGER_INFO(spdlog::get("console"),
                       "Unimplemented length of uniform ({})", values.size());
  }
  }
  CHECK_GL_ERROR();
}

void Program::SetUniform(std::string uniform, glm::mat4 mat, bool camera) {
  CHECK_GL_ERROR();
  if (!uniforms.contains(uniform)) {
    SPDLOG_LOGGER_INFO(spdlog::get("console"), "Uniform not found: {}",
                       uniform);
    return;
  }
  glBindBuffer(GL_UNIFORM_BUFFER, uniforms[uniform]->info.id);
  CHECK_GL_ERROR();

  glBufferSubData(GL_UNIFORM_BUFFER, GetUniformOffset(uniform, camera),
                  sizeof(glm::mat4), glm::value_ptr(mat));
  CHECK_GL_ERROR();
}
void Engine::Graphics::Program::BindTexture2D(Texture tex) {
  glBindTexture(GL_TEXTURE_2D, tex.texture);
}

unsigned int Program::GetUniformOffset(std::string uniform, bool camera) {
  unsigned int offset = 0;
  if (camera)
    offset = 128;
  for (auto [key, val] : uniforms_info) {
    if (val == uniform)
      return offset;
    offset += uniforms[val]->info.offset;
  }
  return -1;
}
