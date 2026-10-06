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
  Keybind(std::string name, int default_key, int key, KeypressType type,
          std::function<void()> func);
  void Trigger();
  json ToJson() override;

  void FromJson(json &js) override;

  bool isEnabled;
  int key;
  int scancode;
  int default_key;
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
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(KeybindJson, name, default_key, key,
                                   scancode)

struct KeybindEvent {
public:
  int scancode;
  KeypressType type;
  bool operator==(const KeybindEvent &rhs) {
    return scancode == rhs.scancode && type == rhs.type;
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
                                             int key, KeypressType type,
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
  std::map<std::string, std::shared_ptr<Keybind>> keybinds = {};
  // TODO: Should make it so that one scancode can be assigned to multiple
  // keybinds - probably through
  // std::map<int, std::vector<std::shared_ptr<Keybind>>>
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
