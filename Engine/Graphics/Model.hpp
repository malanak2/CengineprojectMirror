#pragma once
#include "Graphics/Program.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Texture.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#define MAX_BONE_INFLUENCE 4
#define MAX_BONE_WEIGHTS 500
/// Mesh logic in this file is from https://learnopengl.com/Model-Loading/Mesh
/// Model logic in this file is from https://learnopengl.com/Model-Loading/Model
/// and bone logic is by
/// https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation
namespace Engine {
namespace Graphics {
struct MVertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
  // tangent
  glm::vec3 Tangent;
  // bitangent
  glm::vec3 Bitangent;

  // bone indexes which will influence this vertex
  int m_BoneIDs[MAX_BONE_INFLUENCE];
  // weights from each bone
  float m_Weights[MAX_BONE_INFLUENCE];
};
struct BoneInfo {
  /*id is index in finalBoneMatrices*/
  int id;

  /*offset matrix transforms vertex from model space to bone space*/
  glm::mat4 offset;
};
struct MTexture {
  std::shared_ptr<Texture> texture;
  std::string type;
};
class Mesh {
public:
  // mesh data
  std::string name;
  std::vector<MVertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<MTexture> textures;

  Mesh(std::string name, std::vector<MVertex> vertices,
       std::vector<unsigned int> indices, std::vector<MTexture> textures);
  Mesh(std::vector<MVertex> vertices, std::vector<unsigned int> indices,
       std::vector<MTexture> textures);
  void Draw(Engine::Graphics::Program &program);
  void DrawInstanced(Engine::Graphics::Program &program,
                     unsigned int instanceCount);

  unsigned int GetVAO() const { return VAO; }
  unsigned int GetIndexCount() const {
    return static_cast<unsigned int>(indices.size());
  }

private:
  //  render data
  unsigned int VAO = 0, VBO = 0, EBO = 0;

  void setupMesh();
};
class Model {
public:
  static std::shared_ptr<Model> Create(const std::string &path);
  static std::unordered_map<std::string, std::shared_ptr<Model>> models;

  Model(const std::string &path);
  Model() = default;
  void Draw(Engine::Graphics::Program &program);
  void DrawInstanced(Engine::Graphics::Program &program,
                     unsigned int instanceCount,
                     const std::vector<bool> &enabledMeshes = {});

  const std::vector<Mesh> &GetMeshes() const { return meshes; }
  std::string path;

  std::map<std::string, BoneInfo> m_BoneInfoMap;
  int m_BoneCounter = 0;

  auto &GetBoneInfoMap() { return m_BoneInfoMap; }
  int &GetBoneCount() { return m_BoneCounter; }

  void SetVertexBoneDataToDefault(MVertex &vertex);

  void SetVertexBoneData(MVertex &vertex, int boneID, float weight);

  void ExtractBoneWeightForVertices(std::vector<MVertex> &vertices,
                                    aiMesh *mesh, const aiScene *scene);

private:
  // model data
  std::vector<Mesh> meshes;
  std::string directory;

  void loadModel(std::string path);
  void processNode(aiNode *node, const aiScene *scene);
  Mesh processMesh(aiMesh *mesh, const aiScene *scene);
  std::vector<MTexture> loadMaterialTextures(aiMaterial *mat,
                                             aiTextureType type,
                                             std::string typeName);
};
struct KeyPosition {
  glm::vec3 position;
  float timeStamp;
};

struct KeyRotation {
  glm::quat orientation;
  float timeStamp;
};

struct KeyScale {
  glm::vec3 scale;
  float timeStamp;
};

class Bone {
private:
  std::vector<KeyPosition> m_Positions;
  std::vector<KeyRotation> m_Rotations;
  std::vector<KeyScale> m_Scales;
  int m_NumPositions;
  int m_NumRotations;
  int m_NumScalings;

  glm::mat4 m_LocalTransform;
  std::string m_Name;
  int m_ID;

  int m_LastPositionIndex = 0;
  int m_LastRotationIndex = 0;
  int m_LastScaleIndex = 0;

public:
  /*reads keyframes from aiNodeAnim*/
  Bone(const std::string &name, int ID, const aiNodeAnim *channel);

  /*interpolates  b/w positions,rotations & scaling keys based on the curren
  time of the animation and prepares the local transformation matrix by
  combining all keys tranformations*/
  void Update(float animationTime);

  glm::mat4 GetLocalTransform() { return m_LocalTransform; }
  std::string GetBoneName() const { return m_Name; }
  int GetBoneID() { return m_ID; }

  /* Gets the current index on mKeyPositions to interpolate to based on
  the current animation time*/
  int GetPositionIndex(float animationTime);

  /* Gets the current index on mKeyRotations to interpolate to based on the
  current animation time*/
  int GetRotationIndex(float animationTime);

  /* Gets the current index on mKeyScalings to interpolate to based on the
  current animation time */
  int GetScaleIndex(float animationTime);

  /* Gets normalized value for Lerp & Slerp*/
  float GetScaleFactor(float lastTimeStamp, float nextTimeStamp,
                       float animationTime);

  /*figures out which position keys to interpolate b/w and performs the
  interpolation and returns the translation matrix*/
  glm::mat4 InterpolatePosition(float animationTime);

  /*figures out which rotations keys to interpolate b/w and performs the
  interpolation and returns the rotation matrix*/
  glm::mat4 InterpolateRotation(float animationTime);

  /*figures out which scaling keys to interpolate b/w and performs the
  interpolation and returns the scale matrix*/
  glm::mat4 InterpolateScaling(float animationTime);
};
struct AssimpNodeData {
  glm::mat4 transformation{1.0f};
  std::string name;
  int childrenCount = 0;
  std::vector<AssimpNodeData> children;
  Bone *bone = nullptr;
  int boneInfoId = -1;
  glm::mat4 offsetMatrix{1.0f};
};

struct SocketAttachment {
  std::string targetNode;
  std::string targetBone;
  glm::vec3 offsetPosition = glm::vec3(0.0f);
  glm::vec3 offsetRotation = glm::vec3(0.0f); // Euler in degrees
  glm::vec3 offsetScale = glm::vec3(1.0f);
  bool enabled = true;
};

inline void to_json(nlohmann::json &j, const SocketAttachment &s) {
  j = nlohmann::json{
      {"target_node", s.targetNode},
      {"target_bone", s.targetBone},
      {"position",
       {s.offsetPosition.x, s.offsetPosition.y, s.offsetPosition.z}},
      {"rotation",
       {s.offsetRotation.x, s.offsetRotation.y, s.offsetRotation.z}},
      {"scale", {s.offsetScale.x, s.offsetScale.y, s.offsetScale.z}},
      {"enabled", s.enabled}};
}

inline void from_json(const nlohmann::json &j, SocketAttachment &s) {
  if (j.contains("target_node"))
    j.at("target_node").get_to(s.targetNode);
  if (j.contains("target_bone"))
    j.at("target_bone").get_to(s.targetBone);
  if (j.contains("position") && j["position"].is_array() &&
      j["position"].size() >= 3) {
    s.offsetPosition =
        glm::vec3(j["position"][0], j["position"][1], j["position"][2]);
  }
  if (j.contains("rotation") && j["rotation"].is_array() &&
      j["rotation"].size() >= 3) {
    s.offsetRotation =
        glm::vec3(j["rotation"][0], j["rotation"][1], j["rotation"][2]);
  }
  if (j.contains("scale") && j["scale"].is_array() &&
      j["scale"].size() >= 3) {
    s.offsetScale =
        glm::vec3(j["scale"][0], j["scale"][1], j["scale"][2]);
  }
  if (j.contains("enabled"))
    j.at("enabled").get_to(s.enabled);
}

class Animation {
public:
  Animation() = default;

  Animation(const std::string &animationPath, std::shared_ptr<Model> model,
            int animationIndex = 0);

  ~Animation() {}

  Bone *FindBone(const std::string &name);
  inline const std::string &GetName() const { return m_Name; }
  inline float GetTicksPerSecond() { return m_TicksPerSecond; }

  inline float GetDuration() { return m_Duration; }

  inline const AssimpNodeData &GetRootNode() { return m_RootNode; }

  inline const std::map<std::string, BoneInfo> &GetBoneIDMap() {
    return m_BoneInfoMap;
  }
  inline const std::vector<glm::mat4> &GetDefaultBoneMatrices() const {
    return m_DefaultBoneMatrices;
  }

  void SetSockets(const std::vector<SocketAttachment> &sockets);
  const std::vector<SocketAttachment> &GetSockets() const { return m_Sockets; }
  void RecalculateRestPose();
  const AssimpNodeData *FindNode(const std::string &name) const;
  std::vector<std::string> GetNodeNames() const;

  void CalculateRestPose(const AssimpNodeData *node,
                         const glm::mat4 &parentTransform,
                         std::vector<glm::mat4> &outMatrices) const;

private:
  void CalculateRestPoseInternal(
      const AssimpNodeData *node, const glm::mat4 &parentTransform,
      std::vector<glm::mat4> &outMatrices,
      std::unordered_map<std::string, glm::mat4> &globalTransforms,
      const std::unordered_map<std::string, const SocketAttachment *>
          &activeSockets) const;
  void CalculateSocketSubtreeRestPose(
      const AssimpNodeData *node, const glm::mat4 &parentTransform,
      std::vector<glm::mat4> &outMatrices,
      std::unordered_map<std::string, glm::mat4> &globalTransforms,
      bool isSocketRoot) const;

  void ReadMissingBones(const aiAnimation *animation, Model &model);

  void ReadHeirarchyData(AssimpNodeData &dest, const aiNode *src);
  void SetupNodeHierarchy(AssimpNodeData &node);
  std::string m_Name;
  float m_Duration;
  int m_TicksPerSecond;
  std::vector<Bone> m_Bones;
  AssimpNodeData m_RootNode;
  std::map<std::string, BoneInfo> m_BoneInfoMap;
  std::vector<glm::mat4> m_DefaultBoneMatrices;
  std::vector<SocketAttachment> m_Sockets;
};

class Animator {
public:
  Animator(std::shared_ptr<Animation> currentAnimation);

  void UpdateAnimation(float dt);

  void PlayAnimation(std::shared_ptr<Animation> pAnimation);

  void CalculateBoneTransform(const AssimpNodeData *node,
                              const glm::mat4 &parentTransform);

  const std::vector<glm::mat4> &GetFinalBoneMatrices() const {
    return m_FinalBoneMatrices;
  }

  void SetSockets(const std::vector<SocketAttachment> &sockets);
  const std::vector<SocketAttachment> &GetSockets() const { return m_Sockets; }
  glm::mat4 GetBoneGlobalTransform(const std::string &boneName) const;
  const std::unordered_map<std::string, glm::mat4> &
  GetGlobalNodeTransforms() const {
    return m_GlobalNodeTransforms;
  }

private:
  void CalculateBoneTransformInternal(
      const AssimpNodeData *node, const glm::mat4 &parentTransform,
      const std::unordered_map<std::string, const SocketAttachment *>
          &activeSockets);
  void CalculateSocketSubtree(const AssimpNodeData *node,
                              const glm::mat4 &parentTransform,
                              bool isSocketRoot);

  std::vector<glm::mat4> m_FinalBoneMatrices;
  std::shared_ptr<Animation> m_CurrentAnimation;
  float m_CurrentTime;
  float m_DeltaTime;
  std::vector<SocketAttachment> m_Sockets;
  std::unordered_map<std::string, glm::mat4> m_GlobalNodeTransforms;
};
} // namespace Graphics
} // namespace Engine
