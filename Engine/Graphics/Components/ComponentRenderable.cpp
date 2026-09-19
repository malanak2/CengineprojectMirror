#include "ComponentRenderable.hpp"
#include "Graphics/Graphics.hpp"
#include "Interfaces/IComponent.hpp"
#include "JsonFileBase.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include <memory>
#include <spdlog/spdlog.h>
using namespace Engine::Graphics;

ComponentRenderable::ComponentRenderable(std::string path,
                                         std::shared_ptr<Object> object) {
  this->path = path;
  this->object = object;
  SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Nope");
  // Load();
  // not components
}
/*
std::shared_ptr<ComponentRenderable> ComponentRenderable::Create(
    std::string material_path,
    std::map<std::string, std::vector<float>> uniforms) {
  auto cr = std::make_shared<ComponentRenderable>();
  cr->_material_path = material_path;
  std::shared_ptr<Material> m =
      std::static_pointer_cast<Material>(cr->FromData(material_path, uniforms));
  m->renderableObjects.insert(m->renderableObjects.end(), cr);
  return cr;
};*/

void ComponentRenderable::FromData(
    std::string material_path,
    std::map<std::string, std::shared_ptr<IUniform>> uniforms) {
  material = Material::Create(material_path);
  _material_path.reserve(100);
  auto logger = spdlog::get("console");
  if (!material->usable) {
    SPDLOG_LOGGER_ERROR(logger, "Failed to load material at {}", material_path);
    return;
  }
  for (auto &[key, val] : material->program->uniforms) {
    if (!uniforms.contains(key) && key != "camera") {
      SPDLOG_LOGGER_WARN(
          spdlog::get("console"),
          "Component at {} doesnt contain uniform {} specified in material",
          "%instantiated%", key);
    }
  }
  for (auto &[key, val] : uniforms) {
    if (!material->program->uniforms.contains(key) && key != "camera") {
      SPDLOG_LOGGER_WARN(spdlog::get("console"),
                         "Component at {} specifies uniform {} that is not "
                         "specified in material",
                         "%instantiated%", key);
    } else if (material->program->uniforms.contains(key)) {
      if (!val) {
        val = material->program->uniforms[key];
      } else if (material->program->uniforms[key]) {
        val->info.id = material->program->uniforms[key]->info.id;
        val->info.offset = material->program->uniforms[key]->info.offset;
        if (!val->info.name || val->info.name->empty()) {
          val->info.name = material->program->uniforms[key]->info.name;
        }
      }
    }
  }
  this->_uniforms = uniforms;
}

void ComponentRenderable::Setup() {};
void ComponentRenderable::Update() {};
void ComponentRenderable::FixedUpdate() {}
void Engine::Graphics::ComponentRenderable::SetObject(
    std::shared_ptr<Object> object) {
  this->object = object;
};

json ComponentRenderable::ToJson() {
  JsonFileBase jb;
  RenderableDataJson j;
  j.material_path = _material_path;
  j.indices = indices;
  j.vertices = vertices;
  j.uniforms = this->_uniforms;
  jb.object_type = ObjectType::Component;
  jb.data = j;
  return jb;
}

std::shared_ptr<ComponentRenderable>
ComponentRenderable::Create(json &js, std::shared_ptr<Object> object) {
  JsonFileBase jb;
  jb = js;
  if (jb.object_type != ObjectType::Component) {
    SPDLOG_LOGGER_ERROR(spdlog::get("console"),
                        "Tried to load a component::renderable from a json "
                        "file of different object_type. Path: {}",
                        (std::string)js["data"]["path"]);
  }
  RenderableDataJson json_inst;
  json_inst = js["data"];
  std::shared_ptr<ComponentRenderable> cr =
      std::make_shared<ComponentRenderable>();
  cr->SetObject(object);
  cr->_material_path = json_inst.material_path;
  cr->FromData(json_inst.material_path, json_inst.uniforms);
  cr->material->renderableObjects.insert(cr->material->renderableObjects.end(),
                                         cr);

  glUseProgram(cr->material->program->id);
  CHECK_GL_ERROR();
  std::vector<float> vertices = json_inst.vertices; /*{
       -0.5f, -0.5f, 0.0f, // left
       0.5f,  -0.5f, 0.0f, // right
       0.0f,  0.5f,  0.0f  // top
   };*/

  std::vector<int> indices = json_inst.indices;
  unsigned int VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and
  // then configure vertex attributes(s).
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &(vertices[0]),
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int),
               &(indices[0]), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  bool has_uv = !cr->material->texture_path.empty();

  if (has_uv) {
    // Attribute 0: float3 aPos : POSITION
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    // Attribute 1: float2 aUV : TEXCOORD0
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
  } else {
    // Attribute 0: float3 aPos : POSITION
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
  }
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
  cr->vertices = vertices;
  cr->indices = indices;
  cr->vao = VAO;
  cr->vbo = VBO;
  cr->ebo = EBO;
  CHECK_GL_ERROR();
  return cr;
};

void ComponentRenderable::FromJson(json &js) {
  SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER,
                     "Do not call FromJson, call Create instead");
  JsonFileBase file_base;
  file_base = js;
  if (file_base.object_type != ObjectType::Component) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                        "Tried to load a component::renderable from a json "
                        "file of different object_type.");
    return;
  }
  RenderableDataJson json_inst;
  json_inst = js["data"];
  this->_material_path = json_inst.material_path;
  this->_uniforms = json_inst.uniforms;
  this->vertices = json_inst.vertices;
  this->indices = json_inst.indices;
  _material_path = json_inst.material_path;

  FromData(json_inst.material_path, json_inst.uniforms);
  material->renderableObjects.insert(material->renderableObjects.end(),
                                     shared_from_this());

  glUseProgram(material->program->id);
  CHECK_GL_ERROR();
  std::vector<float> vertices = json_inst.vertices; /*{
       -0.5f, -0.5f, 0.0f, // left
       0.5f,  -0.5f, 0.0f, // right
       0.0f,  0.5f,  0.0f  // top
   };*/

  std::vector<int> indices = json_inst.indices;
  unsigned int VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and
  // then configure vertex attributes(s).
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &(vertices[0]),
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int),
               &(indices[0]), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  bool has_uv = !material->texture_path.empty();

  if (has_uv) {
    // Attribute 0: float3 aPos : POSITION
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    // Attribute 1: float2 aUV : TEXCOORD0
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
  } else {
    // Attribute 0: float3 aPos : POSITION
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
  }
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
  vertices = vertices;
  indices = indices;
  vao = VAO;
  vbo = VBO;
  ebo = EBO;
  CHECK_GL_ERROR();
}

ComponentRenderable::ComponentRenderable(json &js) { FromJson(js); }

ComponentRenderable::ComponentRenderable() {}
