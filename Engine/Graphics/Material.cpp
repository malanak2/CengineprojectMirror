#include "Material.hpp"
#include "Engine.hpp"
#include "Graphics.hpp"
#include "Program.hpp"
#include "Scene.hpp"
#include "Shader.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include "glad/glad.h"
#include "nlohmann/json.hpp" // IWYU pragma: keep
#include "tracy/TracyOpenGL.hpp"
#include <exception>
#include <glm/trigonometric.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <tracy/Tracy.hpp>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stacktrace>
namespace Engine::Graphics {

using json = nlohmann::json;

Material::Material() { throw std::logic_error("Function not implemented"); }
bool Material::ran_from_create = false;
Material::Material(std::string path) {
  ZoneScoped;
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
  texture_path.reserve(100);
  if (m.texture_path == "") {
    texture_path = "";
  } else {
    texture_path = Config::inst->graphics->texturePath + "/" + m.texture_path;
  }

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
        m.uniforms, shaders, texture_path, uses_camera);
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
    SPDLOG_LOGGER_CRITICAL(ENGINE_UTIL_LOGGER, "Stacktrace: {}",
                           std::to_string(std::stacktrace::current()));

    return;
  }
  CHECK_GL_ERROR();
}

void Material::SetupMaterial() {
  ZoneScopedN("Material::Setup");
  TracyGpuZone("Material::Setup");
  program->Setup();
}

void Material::RenderObjects() {
  ZoneScopedN("Material::RenderObjects");
  TracyGpuZone("Material::RenderObjects");

  if (uses_camera) {
    ZoneScopedN("UploadCamera");
    TracyGpuZone("UploadCamera");
    auto scene = Engine::Engine::instance
                     ? Engine::Engine::instance->current_scene
                     : nullptr;
    if (scene && scene->camera && program->uniforms.contains("camera")) {
      auto proj = scene->camera->GetProjMatrix();
      auto view = scene->camera->GetViewMatrix();
      glBindBuffer(GL_UNIFORM_BUFFER, program->uniforms["camera"]->info.id);
      CHECK_GL_ERROR();

      glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
                      glm::value_ptr(proj));
      glBufferSubData(GL_UNIFORM_BUFFER, 64, sizeof(glm::mat4),
                      glm::value_ptr(view));
      CHECK_GL_ERROR();
    }
  }

  std::map<std::shared_ptr<Model>,
           std::vector<std::shared_ptr<ComponentRenderable>>>
      batches;
  {
    ZoneScopedN("BatchingObjects");
    for (auto element : this->renderableObjects) {
      if (element && element->model) {
        batches[element->model].push_back(element);
      }
    }
  }

  for (auto &[model, instances] : batches) {
    ZoneScopedN("ModelBatch");
    ZoneText(model->path.c_str(), model->path.size());
    TracyGpuZoneTransient(__gpu_batch_zone, model->path.c_str(), true);
    if (instances.empty())
      continue;

    std::vector<glm::mat4> transforms;
    transforms.reserve(instances.size());
    for (auto &inst : instances) {
      glm::mat4 transform = glm::mat4(1.0f);
      auto obj = inst->object.lock();
      if (obj) {
        transform =
            glm::translate(transform, glm::make_vec3(&(obj->_position[0])));
        transform = glm::rotate(transform, glm::radians(obj->_rotation[0]),
                                glm::vec3(1, 0, 0));
        transform = glm::rotate(transform, glm::radians(obj->_rotation[1]),
                                glm::vec3(0, 1, 0));
        transform = glm::rotate(transform, glm::radians(obj->_rotation[2]),
                                glm::vec3(0, 0, 1));
      }
      transforms.push_back(transform);
    }

    {
      ZoneScopedN("UploadUniforms");
      TracyGpuZone("UploadUniforms");
      for (auto &[key, val] : instances[0]->_uniforms) {
        if (key == "camera" || key == "translate")
          continue;
        if (program->uniforms.contains(key)) {
          val->Use(program->GetUniformOffset(key, this->uses_camera));
        }
      }
    }

    {
      ZoneScopedN("UploadInstanceSSBO");
      TracyGpuZone("UploadInstanceSSBO");
      if (instanceSSBO == 0) {
        glGenBuffers(1, &instanceSSBO);
      }
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, instanceSSBO);
      glBufferData(GL_SHADER_STORAGE_BUFFER,
                   transforms.size() * sizeof(glm::mat4), transforms.data(),
                   GL_DYNAMIC_DRAW);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, instanceSSBO);
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
      CHECK_GL_ERROR();
    }

    {
      ZoneScopedN("DrawModel");
      TracyGpuZone("DrawModel");
      model->DrawInstanced(*program,
                           static_cast<unsigned int>(transforms.size()));
      CHECK_GL_ERROR();
    }
  }
}

std::shared_ptr<Material> Material::Create(std::string path) {
  ZoneScoped;
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

Material::~Material() {
  if (instanceSSBO != 0) {
    glDeleteBuffers(1, &instanceSSBO);
    instanceSSBO = 0;
  }
}
} // namespace Engine::Graphics
