#include "Model.hpp"
#include "Graphics/Program.hpp"
#include "Util/LoggerUtil.hpp"
#include "glad/glad.h"
#include "tracy/TracyOpenGL.hpp"
#include <filesystem>
#include <memory>
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
  glEnableVertexAttribArray(3);
  // ids
  glEnableVertexAttribArray(3);
  glVertexAttribIPointer(3, 4, GL_INT, sizeof(MVertex),
                         (void *)offsetof(MVertex, m_BoneIDs));

  // weights
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(MVertex),
                        (void *)offsetof(MVertex, m_Weights));

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
      sanitized_path, aiProcess_Triangulate | aiProcess_GenSmoothNormals);

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
    SetVertexBoneDataToDefault(vertex);
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
  ExtractBoneWeightForVertices(vertices, mesh, scene);

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
void Engine::Graphics::Model::SetVertexBoneDataToDefault(MVertex &vertex) {
  for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
    vertex.m_BoneIDs[i] = -1;
    vertex.m_Weights[i] = 0.0f;
  }
}
void Engine::Graphics::Model::SetVertexBoneData(MVertex &vertex, int boneID,
                                                float weight) {
  for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
    if (vertex.m_BoneIDs[i] < 0) {
      vertex.m_Weights[i] = weight;
      vertex.m_BoneIDs[i] = boneID;
      break;
    }
  }
}
void Engine::Graphics::Model::ExtractBoneWeightForVertices(
    std::vector<MVertex> &vertices, aiMesh *mesh, const aiScene *scene) {
  for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
    int boneID = -1;
    std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
    if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end()) {
      BoneInfo newBoneInfo;
      newBoneInfo.id = m_BoneCounter;
      glm::mat4 mat;
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          mat[j][i] = mesh->mBones[boneIndex]->mOffsetMatrix[i][j];
        }
      }
      newBoneInfo.offset = mat;
      m_BoneInfoMap[boneName] = newBoneInfo;
      boneID = m_BoneCounter;
      m_BoneCounter++;
    } else {
      boneID = m_BoneInfoMap[boneName].id;
    }
    assert(boneID != -1);
    auto weights = mesh->mBones[boneIndex]->mWeights;
    int numWeights = mesh->mBones[boneIndex]->mNumWeights;

    for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
      int vertexId = weights[weightIndex].mVertexId;
      float weight = weights[weightIndex].mWeight;
      assert(vertexId <= vertices.size());
      SetVertexBoneData(vertices[vertexId], boneID, weight);
    }
  }
}
Engine::Graphics::Bone::Bone(const std::string &name, int ID,
                             const aiNodeAnim *channel)
    : m_Name(name), m_ID(ID), m_LocalTransform(1.0f) {
  m_NumPositions = channel->mNumPositionKeys;
  m_Positions.reserve(m_NumPositions);
  for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex) {
    aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
    float timeStamp = channel->mPositionKeys[positionIndex].mTime;
    KeyPosition data;
    data.position = glm::vec3(aiPosition.x, aiPosition.y, aiPosition.z);
    data.timeStamp = timeStamp;
    m_Positions.push_back(data);
  }

  m_NumRotations = channel->mNumRotationKeys;
  m_Rotations.reserve(m_NumRotations);
  for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex) {
    aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
    float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
    KeyRotation data;
    data.orientation = glm::quat(aiOrientation.w, aiOrientation.x,
                                 aiOrientation.y, aiOrientation.z);
    data.timeStamp = timeStamp;
    m_Rotations.push_back(data);
  }

  m_NumScalings = channel->mNumScalingKeys;
  m_Scales.reserve(m_NumScalings);
  for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex) {
    aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
    float timeStamp = channel->mScalingKeys[keyIndex].mTime;
    KeyScale data;
    data.scale = glm::vec3(scale.x, scale.y, scale.z);
    data.timeStamp = timeStamp;
    m_Scales.push_back(data);
  }
}
void Engine::Graphics::Bone::Update(float animationTime) {
  glm::mat4 translation = InterpolatePosition(animationTime);
  glm::mat4 rotation = InterpolateRotation(animationTime);
  glm::mat4 scale = InterpolateScaling(animationTime);
  m_LocalTransform = translation * rotation * scale;
}

int Engine::Graphics::Bone::GetPositionIndex(float animationTime) {
  if (m_NumPositions <= 1)
    return 0;
  if (m_LastPositionIndex < m_NumPositions - 1 &&
      animationTime >= m_Positions[m_LastPositionIndex].timeStamp &&
      animationTime < m_Positions[m_LastPositionIndex + 1].timeStamp) {
    return m_LastPositionIndex;
  }
  if (m_LastPositionIndex + 1 < m_NumPositions - 1 &&
      animationTime >= m_Positions[m_LastPositionIndex + 1].timeStamp &&
      animationTime < m_Positions[m_LastPositionIndex + 2].timeStamp) {
    return ++m_LastPositionIndex;
  }
  auto it = std::upper_bound(
      m_Positions.begin(), m_Positions.end() - 1, animationTime,
      [](float val, const KeyPosition &k) { return val < k.timeStamp; });
  m_LastPositionIndex =
      std::max(0, static_cast<int>(std::distance(m_Positions.begin(), it) - 1));
  return m_LastPositionIndex;
}
int Engine::Graphics::Bone::GetRotationIndex(float animationTime) {
  if (m_NumRotations <= 1)
    return 0;
  if (m_LastRotationIndex < m_NumRotations - 1 &&
      animationTime >= m_Rotations[m_LastRotationIndex].timeStamp &&
      animationTime < m_Rotations[m_LastRotationIndex + 1].timeStamp) {
    return m_LastRotationIndex;
  }
  if (m_LastRotationIndex + 1 < m_NumRotations - 1 &&
      animationTime >= m_Rotations[m_LastRotationIndex + 1].timeStamp &&
      animationTime < m_Rotations[m_LastRotationIndex + 2].timeStamp) {
    return ++m_LastRotationIndex;
  }
  auto it = std::upper_bound(
      m_Rotations.begin(), m_Rotations.end() - 1, animationTime,
      [](float val, const KeyRotation &k) { return val < k.timeStamp; });
  m_LastRotationIndex =
      std::max(0, static_cast<int>(std::distance(m_Rotations.begin(), it) - 1));
  return m_LastRotationIndex;
}
int Engine::Graphics::Bone::GetScaleIndex(float animationTime) {
  if (m_NumScalings <= 1)
    return 0;
  if (m_LastScaleIndex < m_NumScalings - 1 &&
      animationTime >= m_Scales[m_LastScaleIndex].timeStamp &&
      animationTime < m_Scales[m_LastScaleIndex + 1].timeStamp) {
    return m_LastScaleIndex;
  }
  if (m_LastScaleIndex + 1 < m_NumScalings - 1 &&
      animationTime >= m_Scales[m_LastScaleIndex + 1].timeStamp &&
      animationTime < m_Scales[m_LastScaleIndex + 2].timeStamp) {
    return ++m_LastScaleIndex;
  }
  auto it = std::upper_bound(
      m_Scales.begin(), m_Scales.end() - 1, animationTime,
      [](float val, const KeyScale &k) { return val < k.timeStamp; });
  m_LastScaleIndex =
      std::max(0, static_cast<int>(std::distance(m_Scales.begin(), it) - 1));
  return m_LastScaleIndex;
}
float Engine::Graphics::Bone::GetScaleFactor(float lastTimeStamp,
                                             float nextTimeStamp,
                                             float animationTime) {
  float scaleFactor = 0.0f;
  float midWayLength = animationTime - lastTimeStamp;
  float framesDiff = nextTimeStamp - lastTimeStamp;
  scaleFactor = midWayLength / framesDiff;
  return scaleFactor;
}
glm::mat4 Engine::Graphics::Bone::InterpolatePosition(float animationTime) {
  if (1 == m_NumPositions)
    return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

  int p0Index = GetPositionIndex(animationTime);
  int p1Index = p0Index + 1;
  float scaleFactor =
      GetScaleFactor(m_Positions[p0Index].timeStamp,
                     m_Positions[p1Index].timeStamp, animationTime);
  glm::vec3 finalPosition =
      glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position,
               scaleFactor);
  return glm::translate(glm::mat4(1.0f), finalPosition);
}
glm::mat4 Engine::Graphics::Bone::InterpolateRotation(float animationTime) {
  if (1 == m_NumRotations) {
    auto rotation = glm::normalize(m_Rotations[0].orientation);
    return glm::mat4_cast(rotation);
  }

  int p0Index = GetRotationIndex(animationTime);
  int p1Index = p0Index + 1;
  float scaleFactor =
      GetScaleFactor(m_Rotations[p0Index].timeStamp,
                     m_Rotations[p1Index].timeStamp, animationTime);
  glm::quat finalRotation =
      glm::slerp(m_Rotations[p0Index].orientation,
                 m_Rotations[p1Index].orientation, scaleFactor);
  finalRotation = glm::normalize(finalRotation);
  return glm::mat4_cast(finalRotation);
}
glm::mat4 Engine::Graphics::Bone::InterpolateScaling(float animationTime) {
  if (1 == m_NumScalings)
    return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

  int p0Index = GetScaleIndex(animationTime);
  int p1Index = p0Index + 1;
  float scaleFactor = GetScaleFactor(
      m_Scales[p0Index].timeStamp, m_Scales[p1Index].timeStamp, animationTime);
  glm::vec3 finalScale =
      glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale, scaleFactor);
  return glm::scale(glm::mat4(1.0f), finalScale);
}
Engine::Graphics::Animator::Animator(
    std::shared_ptr<Animation> currentAnimation) {
  m_CurrentTime = 0.0;
  m_CurrentAnimation = currentAnimation;

  m_FinalBoneMatrices.reserve(500);

  for (int i = 0; i < 500; i++)
    m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
}
void Engine::Graphics::Animator::UpdateAnimation(float dt) {
  ZoneScoped;
  m_DeltaTime = dt;
  if (m_CurrentAnimation) {
    m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
    m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
    CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
  }
}
void Engine::Graphics::Animator::PlayAnimation(
    std::shared_ptr<Animation> pAnimation) {
  m_CurrentAnimation = pAnimation;
  m_CurrentTime = 0.0f;
}
void Engine::Graphics::Animator::CalculateBoneTransform(
    const AssimpNodeData *node, const glm::mat4 &parentTransform) {
  glm::mat4 nodeTransform = node->transformation;

  if (node->bone) {
    node->bone->Update(m_CurrentTime);
    nodeTransform = node->bone->GetLocalTransform();
  }

  glm::mat4 globalTransformation = parentTransform * nodeTransform;

  if (node->boneInfoId >= 0) {
    m_FinalBoneMatrices[node->boneInfoId] =
        globalTransformation * node->offsetMatrix;
  }

  for (int i = 0; i < node->childrenCount; i++)
    CalculateBoneTransform(&node->children[i], globalTransformation);
}
Engine::Graphics::Animation::Animation(const std::string &animationPath,
                                       std::shared_ptr<Model> model,
                                       int animationIndex) {
  std::string sanitized_path = (animationPath.rfind("resources/", 0) == 0)
                                   ? animationPath
                                   : ("resources/" + animationPath);
  Assimp::Importer importer;
  const aiScene *scene =
      importer.ReadFile(sanitized_path, aiProcess_Triangulate);
  assert(scene && scene->mRootNode);
  assert(scene->mNumAnimations > (unsigned int)animationIndex);
  auto animation = scene->mAnimations[animationIndex];
  m_Name = animation->mName.C_Str();
  if (m_Name.empty()) {
    m_Name = "Animation_" + std::to_string(animationIndex);
  }
  m_Duration = animation->mDuration;
  m_TicksPerSecond = animation->mTicksPerSecond;
  ReadHeirarchyData(m_RootNode, scene->mRootNode);
  ReadMissingBones(animation, *model);
  SetupNodeHierarchy(m_RootNode);
}
Engine::Graphics::Bone *
Engine::Graphics::Animation::FindBone(const std::string &name) {
  ZoneScoped;
  auto iter =
      std::find_if(m_Bones.begin(), m_Bones.end(), [&](const Bone &Bone) {
        return Bone.GetBoneName() == name;
      });
  if (iter == m_Bones.end())
    return nullptr;
  else
    return &(*iter);
}

void Engine::Graphics::Animation::ReadMissingBones(const aiAnimation *animation,
                                                   Model &model) {
  int size = animation->mNumChannels;

  auto &boneInfoMap =
      model.GetBoneInfoMap(); // getting m_BoneInfoMap from Model class
  int &boneCount =
      model.GetBoneCount(); // getting the m_BoneCounter from Model class

  // reading channels(bones engaged in an animation and their keyframes)
  for (int i = 0; i < size; i++) {
    auto channel = animation->mChannels[i];
    std::string boneName = channel->mNodeName.data;

    if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
      boneInfoMap[boneName].id = boneCount;
      boneCount++;
    }
    m_Bones.push_back(Bone(channel->mNodeName.data,
                           boneInfoMap[channel->mNodeName.data].id, channel));
  }

  m_BoneInfoMap = boneInfoMap;
}
void Engine::Graphics::Animation::ReadHeirarchyData(AssimpNodeData &dest,
                                                    const aiNode *src) {
  assert(src);

  dest.name = src->mName.data;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      dest.transformation[j][i] = src->mTransformation[i][j];
    }
  }
  dest.childrenCount = src->mNumChildren;

  for (int i = 0; i < src->mNumChildren; i++) {
    AssimpNodeData newData;
    ReadHeirarchyData(newData, src->mChildren[i]);
    dest.children.push_back(newData);
  }
}

void Engine::Graphics::Animation::SetupNodeHierarchy(AssimpNodeData &node) {
  node.bone = FindBone(node.name);
  auto it = m_BoneInfoMap.find(node.name);
  if (it != m_BoneInfoMap.end()) {
    node.boneInfoId = it->second.id;
    node.offsetMatrix = it->second.offset;
  } else {
    node.boneInfoId = -1;
  }
  for (auto &child : node.children) {
    SetupNodeHierarchy(child);
  }
}

