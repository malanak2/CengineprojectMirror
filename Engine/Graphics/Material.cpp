#include "Material.hpp"
#include "Engine.hpp"
#include "Graphics.hpp"
#include "Graphics/Components/ComponentAnimator.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
#include "Graphics/Uniforms/UniformFloatVector.hpp"
#include "Physics/Components/ComponentBoundingBox.hpp"
#include "Physics/PhysicsSystem.hpp"
#include "Program.hpp"
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
  uses_animations = m.uses_animations;
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

std::string UniformSignature(const ComponentRenderable &r) {
  std::string sig;
  for (const auto &[key, val] : r._uniforms) {
    if (key == "camera" || key == "translate")
      continue;
    sig += key;
    sig += '\0';
    if (auto v = std::dynamic_pointer_cast<UniformFloatVector>(val)) {
      sig.append(reinterpret_cast<const char *>(v->data.data()),
                 v->data.size() * sizeof(float));
    } else {
      auto addr = reinterpret_cast<std::uintptr_t>(val.get());
      sig.append(reinterpret_cast<const char *>(&addr), sizeof(addr));
    }
    sig += '\0';
  }
  return sig;
}
void Material::RenderObjects() {
  ZoneScopedN("Material::RenderObjects");
  TracyGpuZone("Material::RenderObjects");
  if (renderableObjects.size() == 0) {
    return;
  }
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

  using BatchKey = std::tuple<std::shared_ptr<Model>, std::vector<bool>,
                              std::string, const void *>;
  std::map<BatchKey, std::vector<std::shared_ptr<ComponentRenderable>>> batches;
  {
    ZoneScopedN("BatchingObjects");
    for (auto &element : this->renderableObjects) {
      if (!element || !element->model)
        continue;
      const void *animatorKey = nullptr;
      if (uses_animations) {
        if (auto obj = element->object.lock())
          animatorKey = obj->GetComponent<ComponentAnimator>().get();
      }
      batches[BatchKey{element->model, element->GetEnabledMeshes(),
                       UniformSignature(*element), animatorKey}]
          .push_back(element);
    }
  }

  for (auto &[batchKey, instances] : batches) {
    const auto &model = std::get<0>(batchKey);
    const std::vector<bool> &enabledMeshes = std::get<1>(batchKey);
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
        glm::vec3 position = obj->GetPosition();
        if (auto phys = obj->GetComponent<Physics::ComponentPhysics>())
          position = phys->GetRenderPosition();

        glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 R = glm::mat4_cast(obj->GetRotation());
        glm::mat4 S = glm::scale(glm::mat4(1.0f), obj->scale);

        transform = T * R * S;
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
      size_t requiredSize = transforms.size() * sizeof(glm::mat4);
      if (instanceSSBO == 0) {
        glGenBuffers(1, &instanceSSBO);
      }
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, instanceSSBO);
      if (instanceSSBOAllocatedSize < requiredSize) {
        instanceSSBOAllocatedSize = requiredSize * 2;
        glBufferData(GL_SHADER_STORAGE_BUFFER, instanceSSBOAllocatedSize,
                     nullptr, GL_DYNAMIC_DRAW);
      }
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
                      transforms.size() * sizeof(glm::mat4), transforms.data());
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, instanceSSBO);
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
      CHECK_GL_ERROR();
    }
    if (uses_animations) {
      ZoneScopedN("UploadBoneSSBo");
      auto obj = instances[0]->object.lock();
      if (obj) {
        auto animatorComp = obj->GetComponent<ComponentAnimator>();
        if (animatorComp) {
          const auto &transforms = animatorComp->GetFinalBoneMatrices();
          if (!transforms.empty()) {
            if (boneSSBO == 0) {
              glGenBuffers(1, &boneSSBO);
              glBindBuffer(GL_SHADER_STORAGE_BUFFER, boneSSBO);
              glBufferData(GL_SHADER_STORAGE_BUFFER,
                           transforms.size() * sizeof(glm::mat4),
                           transforms.data(), GL_DYNAMIC_DRAW);
            } else {
              glBindBuffer(GL_SHADER_STORAGE_BUFFER, boneSSBO);
              glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
                              transforms.size() * sizeof(glm::mat4),
                              transforms.data());
            }
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, boneSSBO);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            CHECK_GL_ERROR();
          }
        }
      }
    }
    {
      ZoneScopedN("DrawModel");
      TracyGpuZone("DrawModel");
      model->DrawInstanced(*program,
                           static_cast<unsigned int>(transforms.size()),
                           enabledMeshes);
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
  if (boneSSBO != 0) {
    glDeleteBuffers(1, &boneSSBO);
    boneSSBO = 0;
  }
}
} // namespace Engine::Graphics
