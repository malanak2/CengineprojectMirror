#include "Material.hpp"
#include "../../Util/FileUtil.hpp"
#include "Engine/Graphics/MaterialJson.hpp"
#include "Graphics.hpp"
#include "Program.hpp"
#include "Shader.hpp"
#include "glad/glad.h"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <cpptrace/basic.hpp>
#include <exception>
#include <memory>
#include <spdlog/spdlog-inl.h>
#include <stdexcept>
#include <vector>

using namespace Graphics;

using json = nlohmann::json;

Material::Material() { throw new std::logic_error("Function not implemented"); }

Material::Material(std::string path) {
  auto logger = spdlog::get("console");
  std::string file;
  if (FileUtil::ReadFile(path, &file) != 0) {
    logger->warn("Could not open material {}", path);
    return;
  }
  json data = json::parse(file);
  if (data["object_type"] != "material") {
    logger->warn("Tried to open {} as material (path: {})",
                 (std::string)data["object_type"], path);
    return;
  }
  MaterialJson m;
  try {
    m = data;
  } catch (const std::exception &e) {
    logger->error("Failed to parse material json at {}", path);
    return;
  }
  this->path = path;
  CHECK_GL_ERROR();
  try {
    std::vector<std::shared_ptr<Shader>> shaders = {};
    auto data_shaders = m.shaders;
    std::for_each(data_shaders.begin(), data_shaders.end(),
                  [&shaders, path](auto &data_shader) {
                    Shader::ShaderType type = Shader::ShaderType::Invalid;
                    if (data_shader.type == "vertex") {
                      type = Shader::ShaderType::Vertex;
                    } else if (data_shader.type == "fragment") {
                      type = Shader::ShaderType::Fragment;
                    }
                    std::shared_ptr<Shader> shader = Shader::Create(
                        type, data_shader.path, data_shader.entrypoint, false);
                    if (shader->isValid) {
                      shaders.insert(shaders.end(), shader);
                    } else {
                      throw std::invalid_argument(
                          "Invalid shader specified in material at " + path +
                          ", shader path is " + data_shader.path);
                    }
                  });
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
    logger->error("Failed to parse material at {}. ({})", path, e.what());
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
