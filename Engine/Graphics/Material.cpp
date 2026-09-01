#include "Material.hpp"
#include "../../Util/FileUtil.hpp"
#include "Graphics.hpp"
#include "Program.hpp"
#include "Shader.hpp"
#include "Util/LoggerUtil.hpp"
#include "glad/glad.h"
#include "nlohmann/json.hpp" // IWYU pragma: keep
#include <cpptrace/basic.hpp>
#include <exception>
#include <glm/trigonometric.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
namespace Engine::Graphics {

using json = nlohmann::json;

Material::Material() { throw std::logic_error("Function not implemented"); }
bool Material::ran_from_create = false;
Material::Material(std::string path) {
  if (!ran_from_create) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                        "DO NOT CALL THIS CONSTRUCTOR FROM OUTSIDE OF "
                        "Material::Create!!!!!!!!!!!!");
  }
  auto logger = spdlog::get("console");
  std::string file;
  if (FileUtil::ReadFile(path, &file) != 0) {
    SPDLOG_LOGGER_WARN(logger, "Could not open material at {}", path);
    return;
  }
  json data = json::parse(file);
  if (data["object_type"] != "material") {
    SPDLOG_LOGGER_WARN(logger, "Tried to open {} as material (path: {})",
                       static_cast<std::string>(data["object_type"]), path);
    return;
  }
  MaterialJson m;
  try {
    m = data;
  } catch (const std::exception &e) {
    SPDLOG_LOGGER_ERROR(logger, "Failed to parse material json at {}", path);
    return;
  }
  this->path = path;
  CHECK_GL_ERROR();
  uses_camera = m.uses_camera;
  try {
    std::vector<std::shared_ptr<Shader>> shaders = {};
    auto data_shaders = m.shaders;
    for (auto &data_shader : data_shaders) {
      std::shared_ptr<Shader> shader = Shader::Create(
          data_shader.type, data_shader.path, data_shader.entrypoint, false);
      if (shader->isValid) {
        shaders.insert(shaders.end(), shader);
      } else {
        throw std::invalid_argument("Invalid shader specified in material at " +
                                    path + ", shader path is " +
                                    data_shader.path);
      }
    }
    std::shared_ptr<Program> program = std::make_shared<Program>(
        m.uniforms, shaders, m.texture_path, m.uses_camera);
    if (!program->isValid) {
      throw std::invalid_argument("Compiled program for material at " + path +
                                  " is invalid.");
    }
    this->program = program;
    this->usable = true;
    CHECK_GL_ERROR();
  } catch (const std::exception &e) {
    SPDLOG_LOGGER_ERROR(logger, "Failed to parse material at {}. ({})", path,
                        e.what());
    cpptrace::generate_trace().print();
    return;
  }
  CHECK_GL_ERROR();
}

void Material::SetupMaterial() { glUseProgram(program->id); }

void Material::RenderObjects() {
  for (auto element : this->renderableObjects) {
    glBindVertexArray(element->vao);
    if (uses_camera) {
      auto obj = element->object.lock();
      if (obj) {
        auto scene = obj->scene.lock();
        if (scene && scene->camera) {
          auto proj = scene->camera->GetProjMatrix();
          auto view = scene->camera->GetViewMatrix();
          glBindBuffer(GL_UNIFORM_BUFFER, program->uniforms["camera"].info.id);
          CHECK_GL_ERROR();

          glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
                          glm::value_ptr(proj));
          glBufferSubData(GL_UNIFORM_BUFFER, 64, sizeof(glm::mat4),
                          glm::value_ptr(view));
          CHECK_GL_ERROR();
        }
      }
    }
    for (auto [key, val] : program->uniforms) {
      if (key == "camera")
        continue;
      if (key == "translate") {
        glm::mat4 transform = glm::mat4(1);
        auto obj = element->object.lock();
        if (obj) {
          transform =
              glm::translate(transform, glm::make_vec3(&(obj->_position[0])));
          transform = glm::rotate(transform, glm::radians(obj->_rotation[0]),
                                  glm::vec3(1, 0, 0));
          transform = glm::rotate(transform, glm::radians(obj->_rotation[1]),
                                  glm::vec3(0, 1, 0));
          transform = glm::rotate(transform, glm::radians(obj->_rotation[2]),
                                  glm::vec3(0, 0, 1));
          program->SetUniform("translate", transform, uses_camera);
        }
        continue;
      }
      program->SetUniform(
          key, *std::static_pointer_cast<float>(element->_uniforms[key].Data),
          uses_camera);
    }
    glDrawElements(GL_TRIANGLES, element->indices.size(), GL_UNSIGNED_INT, 0);
    CHECK_GL_ERROR();
  }
}

std::shared_ptr<Material> Material::Create(std::string path) {
  if (Main::materials.contains(path)) {
    SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Hit cache for material {}", path);
    return Main::materials[path];
  }
  ran_from_create = true;
  auto material = std::make_shared<Material>(path);
  ran_from_create = false;
  if (material->usable) {
    Main::materials[path] = material;
  } else {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                        "Created an unusable material at {}", path);
  }
  return material;
}

Material::~Material() {}
} // namespace Engine::Graphics
