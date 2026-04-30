#include "Material.hpp"
#include "../../Util/FileUtil.hpp"
#include "Engine/Graphics/MaterialJson.hpp"
#include "Graphics.hpp"
#include "Program.hpp"
#include "Shader.hpp"
#include "glad/glad.h"
#include "nlohmann/json.hpp" // IWYU pragma: keep
#include <cpptrace/basic.hpp>
#include <exception>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <vector>


namespace Graphics {

using json = nlohmann::json;

Material::Material() { throw std::logic_error("Function not implemented"); }

Material::Material(std::string path) {
  auto logger = spdlog::get("console");
  std::string file;
  if (FileUtil::ReadFile(path, &file) != 0) {
    SPDLOG_LOGGER_WARN(logger, "Could not open material {}", path);
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
    std::shared_ptr<Program> program =
        std::make_shared<Program>(m.uniforms, shaders);
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
  throw std::logic_error("Function not implemented");
}

std::shared_ptr<Material> Material::Create(std::string path) {
  if (Main::materials.contains(path)) {
    return Main::materials[path];
  }
  auto material = std::make_shared<Material>(path);
  if (material->usable) {
    Main::materials[path] = material;
  }
  return material;
}

Material::~Material() { Main::materials.erase(path); }
} // namespace Graphics
