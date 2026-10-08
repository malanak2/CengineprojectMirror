#pragma once

#include "Interfaces/IJson.hpp"
#include "glad/glad.h"
#include "nlohmann/json.hpp" // IWYU pragma: keep
#include <GLFW/glfw3.h>
#include <fmt/base.h>
#include <map>
#include <memory>
#include <nlohmann/detail/macro_scope.hpp>
namespace Engine {

constexpr int MODIFIER_MASK = GLFW_MOD_SHIFT | GLFW_MOD_CONTROL | GLFW_MOD_ALT;

inline int SanitizeMods(int mods) { return mods & MODIFIER_MASK; }

inline int GetKeyModifierBit(int key) {
  switch (key) {
  case GLFW_KEY_LEFT_SHIFT:
  case GLFW_KEY_RIGHT_SHIFT:
    return GLFW_MOD_SHIFT;
  case GLFW_KEY_LEFT_CONTROL:
  case GLFW_KEY_RIGHT_CONTROL:
    return GLFW_MOD_CONTROL;
  case GLFW_KEY_LEFT_ALT:
  case GLFW_KEY_RIGHT_ALT:
    return GLFW_MOD_ALT;
  default:
    return 0;
  }
}

inline int QueryCurrentMods(GLFWwindow *window) {
  int mods = 0;
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
    mods |= GLFW_MOD_SHIFT;
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
    mods |= GLFW_MOD_CONTROL;
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS) {
    mods |= GLFW_MOD_ALT;
  }
  return mods;
}

enum KeypressType {
  PRESS = GLFW_PRESS,
  HOLD = 10,
  HOLD_TEXT = GLFW_REPEAT,
  RELEASE = GLFW_RELEASE,
  INVALID = 99
};
class Keybind : public IJson {
public:
  //!
  //! @brief Constructor for a Keybind
  //!
  //! @param[in] key The keycode - for example GLFW_KEY_E
  //! @param[in] type If the function should be called on press, release, ...
  //! @param[in] func the function to call
  //!
  Keybind(std::string name, int default_key, int key, int mods,
          KeypressType type, std::function<void()> func);
  void Trigger();
  json ToJson() override;

  void FromJson(json &js) override;

  bool isEnabled;
  bool isHeld = false;
  int key;
  int scancode;
  int default_key;
  int mods;
  KeypressType type;
  std::function<void()> f;
  std::string name;
};

class KeybindJson {
public:
  std::string name;
  int default_key;
  int key;
  int scancode;
  int mods;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(KeybindJson, name, default_key, key,
                                   scancode, mods)

struct KeybindEvent {
public:
  int scancode;
  int mods;
  KeypressType type;
  bool operator==(const KeybindEvent &rhs) const {
    return scancode == rhs.scancode && type == rhs.type &&
           SanitizeMods(mods) == SanitizeMods(rhs.mods);
  };
};

class InputSystem {
public:
  static void Init();
  static void Save();
  static void ProcessEvents();
  //!
  //! @brief Function to create a new keybind and add it to the correct map
  //!
  //! @param[in] id The unique identifier of the keybind
  //! @param[in] default_key The key to default to
  //! @param[in] key The actual key, in the code this should be the default key
  //! @param[in] type The keypress type
  //! @param[in] func Function to call when the keybind is triggerred
  //! @param[in] isInit Should only be set if called from InputSystem::Init() -
  //! this makes it NOT ADD to all the keymaps (used for saving user changes to
  //! keybinds)
  //!
  static std::shared_ptr<Keybind> NewKeybind(std::string id, int default_key,
                                             int key, int mods,
                                             KeypressType type,
                                             std::function<void()> func,
                                             bool isInit = false);
  static std::shared_ptr<Keybind> GetKeybind(std::string id);

  static std::shared_ptr<InputSystem> instance;

  //!
  //! @events The keybind event queue. Each frame, the callback populates it and
  //! at the end, the @ProcessEvents fuction goes through all the keybind events
  //! and then it is cleared
  //!
  std::vector<KeybindEvent> events = {};
  std::vector<int> heldKeys = {};
  int currentMods = 0;
  std::map<std::string, std::shared_ptr<Keybind>> keybinds = {};
  std::map<int, std::vector<std::shared_ptr<Keybind>>> keybind_map_press = {};
  std::map<int, std::vector<std::shared_ptr<Keybind>>> keybind_map_hold = {};
  std::map<int, std::vector<std::shared_ptr<Keybind>>> keybind_map_hold_text =
      {};
  std::map<int, std::vector<std::shared_ptr<Keybind>>> keybind_map_release = {};
};
class InputSystemJson {
public:
  std::vector<KeybindJson> keybindsPress;
  std::vector<KeybindJson> keybindsHold;
  std::vector<KeybindJson> keybindsHoldText;
  std::vector<KeybindJson> keybindsRelease;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(InputSystemJson, keybindsPress, keybindsHold,
                                   keybindsHoldText, keybindsRelease);
} // namespace Engine
