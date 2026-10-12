#pragma once

#include "Interfaces/IJson.hpp"
#include "glad/glad.h"
#include "nlohmann/json.hpp" // IWYU pragma: keep
#include <GLFW/glfw3.h>
#include <fmt/base.h>
#include <imgui_impl_glfw.h>
#include <map>
#include <memory>
#include <nlohmann/detail/macro_scope.hpp>
#include <string>
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

  bool operator==(const Keybind &rhs) const {
    if (name == rhs.name) {
      return true;
    }
    return false;
  }

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
enum KeybindMouseEventType {
  MOVE,
  CLICK,
  MOVE_CLICKED,
  SCROLL,
};
class KeybindMouse : public IJson {
public:
  std::string name;
  int button;
  bool isEnabled;
  virtual void CallFunc(double xposmouse, double yposmouse, int mbutton,
                        int maction, int mmods, double xscrolloffset,
                        double yscrolloffset, double positionX,
                        double positionY) = 0;
  bool operator==(const KeybindMouse &rhs) const { return name == rhs.name; };
  virtual KeybindMouseEventType GetType() = 0;
};
enum MOVE_TYPE {
  NORMAL,
  BUTTON_HELD,
};
class KeybindMouseMove : public KeybindMouse {
public:
  KeybindMouseEventType GetType() override {
    return KeybindMouseEventType::MOVE;
  }

  json ToJson() override { return {}; }

  void FromJson(json &js) override {}
  MOVE_TYPE mtype;
  int keyMove;
  int mmods;

  // xpos, ypos
  std::function<void(double, double)> func;
  void CallFunc(double xposmouse, double yposmouse, int mbutton, int maction,
                int mmods, double xscrolloffset, double yscrolloffset,
                double positionX, double positionY) override;
};
enum CLICK_TYPE { CLICK_PRESS, CLICK_RELEASE };
class KeybindMouseClick : public KeybindMouse {
public:
  KeybindMouseEventType GetType() override {
    return KeybindMouseEventType::CLICK;
  }

  json ToJson() override { return {}; }

  void FromJson(json &js) override {}
  std::function<void()> func;
  CLICK_TYPE ctype;
  int mmods;

  void CallFunc(double xposmouse, double yposmouse, int mbutton, int maction,
                int mmods, double xscrolloffset, double yscrolloffset,
                double positionX, double positionY) override {
    if (func == nullptr || !isEnabled)
      return;
    if (ctype == CLICK_PRESS && maction == GLFW_PRESS && this->mmods == mmods) {
      func();
    } else if (ctype == CLICK_RELEASE && maction == GLFW_RELEASE) {
      func();
    }
  }
};
enum SCROLL_TYPE {
  SCROLL_X_TYPE_UP,
  SCROLL_X_TYPE_DOWN,
  SCROLL_Y_TYPE_UP,
  SCROLL_Y_TYPE_DOWN,
};
class KeybindMouseScroll : public KeybindMouse {
public:
  KeybindMouseEventType GetType() override {
    return KeybindMouseEventType::SCROLL;
  }

  json ToJson() override { return {}; }

  void FromJson(json &js) override {}
  //!
  //! @func Xposoffset/yposoffset, posx/posy
  //!
  std::function<void(double, double)> func;
  SCROLL_TYPE scrolltype;
  void CallFunc(double xposmouse, double yposmouse, int mbutton, int maction,
                int mmods, double xscrolloffset, double yscrolloffset,
                double positionX, double positionY) override {
    if (func == nullptr || !isEnabled)
      return;
    switch (scrolltype) {
    case SCROLL_X_TYPE_UP: {
      if (xscrolloffset > 0) {
        func(xscrolloffset, positionX);
      }
      break;
    }
    case SCROLL_X_TYPE_DOWN: {
      if (xscrolloffset < 0) {
        func(xscrolloffset, positionX);
      }
      break;
    }
    case SCROLL_Y_TYPE_UP: {
      if (yscrolloffset > 0) {
        func(yscrolloffset, positionY);
      }
      break;
    }
    case SCROLL_Y_TYPE_DOWN: {
      if (yscrolloffset < 0) {
        func(yscrolloffset, positionY);
      }
      break;
    }
    }
  }
};
struct KeybindMouseEvent {
public:
  KeybindMouseEventType type;
  int mbutton;

  int action;
  int mods;
  double xscrolloffset;
  double yscrolloffset;
  double xpos;
  double ypos;
  bool operator==(const KeybindMouseEvent &rhs) {
    return type == rhs.type && mbutton == rhs.mbutton &&
           xscrolloffset == rhs.xscrolloffset &&
           yscrolloffset == rhs.yscrolloffset && xpos == rhs.xpos &&
           ypos == rhs.ypos && action == rhs.action && mods == rhs.mods;
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
  static std::shared_ptr<Keybind> ChangeKeybind(std::string id, int old,
                                                int new_key, int mods);
  static std::shared_ptr<Keybind> GetKeybind(std::string id);

  static std::shared_ptr<InputSystem> instance;

  template <typename T> std::shared_ptr<T> NewKeybindMouse() {};

  //!
  //! @events The keybind event queue. Each frame, the callback populates it and
  //! at the end, the @ProcessEvents fuction goes through all the keybind events
  //! and then it is cleared
  //!
  std::vector<KeybindEvent> events = {};
  std::vector<int> heldKeys = {};
  int currentMods = 0;
  static std::vector<std::function<void(GLFWwindow *, int, int, int, int)>>
      onCallback;
  static std::vector<std::function<void(GLFWwindow *, int, int, int)>>
      onMouseButton;
  static std::vector<std::function<void(GLFWwindow *, int, int)>> onCursorMove;
  static std::vector<std::function<void(GLFWwindow *, double, double)>>
      onMouseScroll;

  std::map<std::string, std::shared_ptr<Keybind>> keybinds = {};
  std::map<int, std::vector<std::shared_ptr<Keybind>>> keybind_map_press = {};
  std::map<int, std::vector<std::shared_ptr<Keybind>>> keybind_map_hold = {};
  std::map<int, std::vector<std::shared_ptr<Keybind>>> keybind_map_hold_text =
      {};
  std::map<int, std::vector<std::shared_ptr<Keybind>>> keybind_map_release = {};
  std::vector<std::shared_ptr<KeybindMouse>> keybind_vector_mouse_click = {};
  std::vector<std::shared_ptr<KeybindMouse>> keybind_vector_mouse_move = {};
  std::vector<std::shared_ptr<KeybindMouse>> keybind_vector_mouse_release = {};
  std::vector<std::shared_ptr<KeybindMouse>> keybind_vector_mouse_scroll = {};
  std::vector<KeybindMouseEvent> keybind_mouse_events = {};
  std::vector<int> held_mouse_buttons = {};
  double mouseXScrollOffset = 0;
  double mouseYScrollOffset = 0;
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
