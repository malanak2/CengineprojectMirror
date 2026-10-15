#pragma once

#include "Interfaces/IJson.hpp"
#include "glad/glad.h"
#include "nlohmann/json.hpp" // IWYU pragma: keep
#include <GLFW/glfw3.h>
#include <fmt/base.h>
#include <functional>
#include <map>
#include <memory>
#include <nlohmann/detail/macro_scope.hpp>
#include <string>
#include <type_traits>
#include <vector>
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
inline std::string GetGlfwMouseButtonName(int button) {
  switch (button) {
  case GLFW_MOUSE_BUTTON_LEFT:
    return "Mouse Left";
  case GLFW_MOUSE_BUTTON_RIGHT:
    return "Mouse Right";
  case GLFW_MOUSE_BUTTON_MIDDLE:
    return "Mouse Middle";
  case GLFW_MOUSE_BUTTON_4:
    return "Mouse 4";
  case GLFW_MOUSE_BUTTON_5:
    return "Mouse 5";
  case GLFW_MOUSE_BUTTON_6:
    return "Mouse 6";
  case GLFW_MOUSE_BUTTON_7:
    return "Mouse 7";
  case GLFW_MOUSE_BUTTON_8:
    return "Mouse 8";
  default:
    return "Mouse " + std::to_string(button);
  }
}

enum KeybindMouseEventType {
  MOVE,
  CLICK,
  MOVE_CLICKED,
  SCROLL,
};

class KeybindMouse : public IJson {
public:
  KeybindMouse() = default;
  virtual ~KeybindMouse() = default;

  std::string name;
  int default_button = 0;
  int button = 0;
  int mods = 0;
  bool isEnabled = true;
  bool isHeld = false;

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
  KeybindMouseMove() = default;
  KeybindMouseMove(std::string name, MOVE_TYPE mtype, int default_button,
                   int button, int mods,
                   std::function<void(double, double)> func);

  KeybindMouseEventType GetType() override {
    return KeybindMouseEventType::MOVE;
  }

  json ToJson() override;
  void FromJson(json &js) override;

  MOVE_TYPE mtype = NORMAL;
  // xpos, ypos
  std::function<void(double, double)> func;
  void CallFunc(double xposmouse, double yposmouse, int mbutton, int maction,
                int mmods, double xscrolloffset, double yscrolloffset,
                double positionX, double positionY) override;
};

enum CLICK_TYPE { CLICK_PRESS, CLICK_RELEASE };

class KeybindMouseClick : public KeybindMouse {
public:
  KeybindMouseClick() = default;
  KeybindMouseClick(std::string name, int default_button, int button, int mods,
                    CLICK_TYPE ctype, std::function<void()> func);

  KeybindMouseEventType GetType() override {
    return KeybindMouseEventType::CLICK;
  }

  void Trigger();
  json ToJson() override;
  void FromJson(json &js) override;

  std::function<void()> func;
  CLICK_TYPE ctype = CLICK_PRESS;

  void CallFunc(double xposmouse, double yposmouse, int mbutton, int maction,
                int mmods, double xscrolloffset, double yscrolloffset,
                double positionX, double positionY) override;
};

enum SCROLL_TYPE {
  SCROLL_X_TYPE_UP,
  SCROLL_X_TYPE_DOWN,
  SCROLL_Y_TYPE_UP,
  SCROLL_Y_TYPE_DOWN,
  SCROLL_ANY,
};

class KeybindMouseScroll : public KeybindMouse {
public:
  KeybindMouseScroll() = default;
  KeybindMouseScroll(std::string name, SCROLL_TYPE scrolltype, int mods,
                     std::function<void(double, double)> func);

  KeybindMouseEventType GetType() override {
    return KeybindMouseEventType::SCROLL;
  }

  json ToJson() override;
  void FromJson(json &js) override;

  //!
  //! @func (xposoffset/yposoffset, posx/posy) or (xoffset, yoffset)
  //!
  std::function<void(double, double)> func;
  SCROLL_TYPE scrolltype = SCROLL_Y_TYPE_UP;

  void CallFunc(double xposmouse, double yposmouse, int mbutton, int maction,
                int mmods, double xscrolloffset, double yscrolloffset,
                double positionX, double positionY) override;
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
  bool operator==(const KeybindMouseEvent &rhs) const {
    return type == rhs.type && mbutton == rhs.mbutton &&
           xscrolloffset == rhs.xscrolloffset &&
           yscrolloffset == rhs.yscrolloffset && xpos == rhs.xpos &&
           ypos == rhs.ypos && action == rhs.action && mods == rhs.mods;
  };
};

class KeybindMouseMoveJson {
public:
  std::string name;
  int mtype;
  int default_button;
  int button;
  int mods;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(KeybindMouseMoveJson, name, mtype,
                                   default_button, button, mods)

class KeybindMouseClickJson {
public:
  std::string name;
  int ctype;
  int default_button;
  int button;
  int mods;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(KeybindMouseClickJson, name, ctype,
                                   default_button, button, mods)

class KeybindMouseScrollJson {
public:
  std::string name;
  int scrolltype;
  int mods;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(KeybindMouseScrollJson, name, scrolltype,
                                   mods)

class InputSystemJson {
public:
  std::vector<KeybindJson> keybindsPress;
  std::vector<KeybindJson> keybindsHold;
  std::vector<KeybindJson> keybindsHoldText;
  std::vector<KeybindJson> keybindsRelease;
  std::vector<KeybindMouseMoveJson> mouseMove;
  std::vector<KeybindMouseClickJson> mouseClick;
  std::vector<KeybindMouseClickJson> mouseRelease;
  std::vector<KeybindMouseScrollJson> mouseScroll;
};

inline void to_json(nlohmann::json &j, const InputSystemJson &p) {
  j = nlohmann::json{
      {"keybindsPress", p.keybindsPress},
      {"keybindsHold", p.keybindsHold},
      {"keybindsHoldText", p.keybindsHoldText},
      {"keybindsRelease", p.keybindsRelease},
      {"mouseMove", p.mouseMove},
      {"mouseClick", p.mouseClick},
      {"mouseRelease", p.mouseRelease},
      {"mouseScroll", p.mouseScroll},
  };
}

inline void from_json(const nlohmann::json &j, InputSystemJson &p) {
  if (j.contains("keybindsPress"))
    j.at("keybindsPress").get_to(p.keybindsPress);
  if (j.contains("keybindsHold"))
    j.at("keybindsHold").get_to(p.keybindsHold);
  if (j.contains("keybindsHoldText"))
    j.at("keybindsHoldText").get_to(p.keybindsHoldText);
  if (j.contains("keybindsRelease"))
    j.at("keybindsRelease").get_to(p.keybindsRelease);
  if (j.contains("mouseMove"))
    j.at("mouseMove").get_to(p.mouseMove);
  if (j.contains("mouseClick"))
    j.at("mouseClick").get_to(p.mouseClick);
  if (j.contains("mouseRelease"))
    j.at("mouseRelease").get_to(p.mouseRelease);
  if (j.contains("mouseScroll"))
    j.at("mouseScroll").get_to(p.mouseScroll);
}

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

  static std::shared_ptr<KeybindMouseClick>
  NewKeybindMouseClick(std::string id, int default_button, int button, int mods,
                       CLICK_TYPE type, std::function<void()> func,
                       bool isInit = false);

  static std::shared_ptr<KeybindMouseMove>
  NewKeybindMouseMove(std::string id, MOVE_TYPE mtype, int default_button,
                      int button, int mods,
                      std::function<void(double, double)> func,
                      bool isInit = false);

  static std::shared_ptr<KeybindMouseScroll>
  NewKeybindMouseScroll(std::string id, SCROLL_TYPE scrolltype, int mods,
                        std::function<void(double, double)> func,
                        bool isInit = false);

  template <typename T, typename... Args>
  static std::shared_ptr<T> NewKeybindMouse(Args &&...args) {
    if constexpr (sizeof...(Args) == 0) {
      return std::make_shared<T>();
    } else if constexpr (std::is_same_v<T, KeybindMouseClick>) {
      return NewKeybindMouseClick(std::forward<Args>(args)...);
    } else if constexpr (std::is_same_v<T, KeybindMouseMove>) {
      return NewKeybindMouseMove(std::forward<Args>(args)...);
    } else if constexpr (std::is_same_v<T, KeybindMouseScroll>) {
      return NewKeybindMouseScroll(std::forward<Args>(args)...);
    } else {
      static_assert(sizeof(T) == 0, "Unsupported KeybindMouse type");
    }
  }

  static std::shared_ptr<KeybindMouse> GetKeybindMouse(std::string id);
  static std::shared_ptr<KeybindMouse> ChangeKeybindMouse(std::string id,
                                                          int new_button,
                                                          int mods);

  static double GetMouseX();
  static double GetMouseY();
  static double GetMouseDeltaX();
  static double GetMouseDeltaY();
  static double GetMouseScrollX();
  static double GetMouseScrollY();
  static bool IsMouseButtonHeld(int button);

  static std::shared_ptr<InputSystem> instance;

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
  static std::vector<std::function<void(GLFWwindow *, double, double)>>
      onCursorMove;
  static std::vector<std::function<void(GLFWwindow *, double, double)>>
      onMouseScroll;

  std::map<std::string, std::shared_ptr<Keybind>> keybinds = {};
  std::map<int, std::vector<std::shared_ptr<Keybind>>> keybind_map_press = {};
  std::map<int, std::vector<std::shared_ptr<Keybind>>> keybind_map_hold = {};
  std::map<int, std::vector<std::shared_ptr<Keybind>>> keybind_map_hold_text =
      {};
  std::map<int, std::vector<std::shared_ptr<Keybind>>> keybind_map_release = {};

  std::map<std::string, std::shared_ptr<KeybindMouse>> mouse_keybinds = {};
  std::vector<std::shared_ptr<KeybindMouse>> keybind_vector_mouse_click = {};
  std::vector<std::shared_ptr<KeybindMouse>> keybind_vector_mouse_move = {};
  std::vector<std::shared_ptr<KeybindMouse>> keybind_vector_mouse_release = {};
  std::vector<std::shared_ptr<KeybindMouse>> keybind_vector_mouse_scroll = {};
  std::vector<KeybindMouseEvent> keybind_mouse_events = {};
  std::vector<int> held_mouse_buttons = {};
  double mouseX = 0;
  double mouseY = 0;
  double mouseDeltaX = 0;
  double mouseDeltaY = 0;
  double lastMouseX = 0;
  double lastMouseY = 0;
  bool firstMouseMove = true;
  double mouseXScrollOffset = 0;
  double mouseYScrollOffset = 0;
};

} // namespace Engine
