#include "Model.hpp"
#include "Graphics/Program.hpp"
#include "Util/LoggerUtil.hpp"
#include "glad/glad.h"
#include "tracy/TracyOpenGL.hpp"
#include <filesystem>
#include <tracy/Tracy.hpp>

using namespace Engine::Graphics;

std::unordered_map<std::string, std::shared_ptr<Model>> Model::models = {};

std::shared_ptr<Model> Model::Create(const std::string &path) {
  ZoneScoped;
  ZoneText(path.c_str(), path.size());
  if (models.contains(path)) {
    return models[path];
  }
  auto m = std::make_shared<Model>(path);
  models[path] = m;
  return m;
}

Engine::Graphics::Mesh::Mesh(std::vector<MVertex> vertices,
                             std::vector<unsigned int> indices,
                             std::vector<MTexture> textures) {
  ZoneScoped;
  this->vertices = vertices;
  this->indices = indices;
  this->textures = textures;

  setupMesh();
}

void Engine::Graphics::Mesh::Draw(Engine::Graphics::Program &program) {
  ZoneScopedN("Mesh::Draw");
  TracyGpuZone("Mesh::Draw");
  DrawInstanced(program, 1);
}

void Engine::Graphics::Mesh::DrawInstanced(Engine::Graphics::Program &program,
                                           unsigned int instanceCount) {
  ZoneScopedN("Mesh::DrawInstanced");
  TracyGpuZone("Mesh::DrawInstanced");
  if (instanceCount == 0)
    return;
  if (!textures.empty() && textures[0].texture &&
      textures[0].texture->texture != (unsigned int)-1) {
    ZoneScopedN("BindTexture");
    TracyGpuZone("BindTexture");
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[0].texture->texture);
  }

  {
    ZoneScopedN("glDrawElementsInstanced");
    TracyGpuZone("glDrawElementsInstanced");
    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0,
                            instanceCount);
    glBindVertexArray(0);
  }
}

void Engine::Graphics::Mesh::setupMesh() {
  ZoneScoped;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MVertex), &vertices[0],
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               &indices[0], GL_STATIC_DRAW);

  // vertex positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MVertex), (void *)0);
  // vertex normals
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MVertex),
                        (void *)offsetof(MVertex, Normal));
  // vertex texture coords
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MVertex),
                        (void *)offsetof(MVertex, TexCoords));

  glBindVertexArray(0);
}

Engine::Graphics::Model::Model(const std::string &path) {
  ZoneScoped;
  ZoneText(path.c_str(), path.size());
  this->path = path;
  loadModel(path);
}

void Engine::Graphics::Model::Draw(Engine::Graphics::Program &program) {
  ZoneScopedN("Model::Draw");
  TracyGpuZone("Model::Draw");
  DrawInstanced(program, 1);
}

void Engine::Graphics::Model::DrawInstanced(Engine::Graphics::Program &program,
                                            unsigned int instanceCount) {
  ZoneScopedN("Model::DrawInstanced");
  TracyGpuZone("Model::DrawInstanced");
  for (auto &mesh : meshes) {
    mesh.DrawInstanced(program, instanceCount);
  }
}

void Engine::Graphics::Model::loadModel(std::string path) {
  ZoneScoped;
  ZoneText(path.c_str(), path.size());
  Assimp::Importer import;
  std::string sanitized_path =
      (path.rfind("resources/", 0) == 0) ? path : ("resources/" + path);

  const aiScene *scene = import.ReadFile(
      sanitized_path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                          aiProcess_PreTransformVertices);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "ERROR::ASSIMP::{}",
                        import.GetErrorString());
    return;
  }
  size_t last_slash = path.find_last_of("/\\");
  directory =
      (last_slash != std::string::npos) ? path.substr(0, last_slash) : "";

  processNode(scene->mRootNode, scene);
}

void Engine::Graphics::Model::processNode(aiNode *node, const aiScene *scene) {
  ZoneScoped;
  // process all the node's meshes (if any)
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(processMesh(mesh, scene));
  }
  // then do the same for each of its children
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene);
  }
}

Engine::Graphics::Mesh
Engine::Graphics::Model::processMesh(aiMesh *mesh, const aiScene *scene) {
  ZoneScoped;
  std::vector<MVertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<MTexture> textures;

  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    MVertex vertex;
    glm::vec3 vector;
    vector.x = mesh->mVertices[i].x;
    vector.y = mesh->mVertices[i].y;
    vector.z = mesh->mVertices[i].z;
    vertex.Position = vector;

    if (mesh->HasNormals()) {
      vector.x = mesh->mNormals[i].x;
      vector.y = mesh->mNormals[i].y;
      vector.z = mesh->mNormals[i].z;
      vertex.Normal = vector;
    } else {
      vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    if (mesh->mTextureCoords[0]) {
      glm::vec2 vec;
      vec.x = mesh->mTextureCoords[0][i].x;
      vec.y = mesh->mTextureCoords[0][i].y;
      vertex.TexCoords = vec;
    } else {
      vertex.TexCoords = glm::vec2(0.0f, 0.0f);
    }
    vertices.push_back(vertex);
  }

  // process indices
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      indices.push_back(face.mIndices[j]);
  }

  // process material
  if (mesh->mMaterialIndex >= 0) {
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    std::vector<MTexture> diffuseMaps = loadMaterialTextures(
        material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    std::vector<MTexture> specularMaps = loadMaterialTextures(
        material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
  }

  return Mesh(vertices, indices, textures);
}

std::vector<MTexture> Engine::Graphics::Model::loadMaterialTextures(
    aiMaterial *mat, aiTextureType type, std::string typeName) {
  ZoneScoped;
  std::vector<MTexture> textures;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);
    MTexture texture;
    texture.texture = Texture::CreateModel(str.data);
    texture.type = typeName;
    textures.push_back(texture);
  }
  return textures;
}
