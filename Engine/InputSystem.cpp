#include "InputSystem.hpp"
#include "Config/Config.hpp"
#include "Graphics/Graphics.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <exception>
#include <fmt/base.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

using namespace Engine;
std::vector<std::function<void(GLFWwindow *, int, int, int, int)>>
    InputSystem::onCallback = {};
void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  ZoneScoped;
  auto instance = InputSystem::instance;
  for (auto f : InputSystem::onMouseButton) {
    f(window, button, action, mods);
  }
  auto v = &InputSystem::instance->keybind_mouse_events;
  v->push_back(KeybindMouseEvent{
      KeybindMouseEventType::CLICK,
      button,
      action,
      QueryCurrentMods(window),
      0,
      0,
      0,
      0,
  });
}
void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
  ZoneScoped;
  auto instance = InputSystem::instance;
  for (auto f : InputSystem::onCursorMove) {
    f(window, xpos, ypos);
  }
  auto v = &InputSystem::instance->keybind_mouse_events;
  v->push_back(KeybindMouseEvent{
      KeybindMouseEventType::MOVE,
      0,
      0,
      QueryCurrentMods(window),
      0,
      0,
      xpos,
      ypos,
  });
}
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  ZoneScoped;
  auto instance = InputSystem::instance;
  for (auto f : InputSystem::onMouseScroll) {
    f(window, xoffset, yoffset);
  }
  auto v = &InputSystem::instance->keybind_mouse_events;
  v->push_back(KeybindMouseEvent{
      KeybindMouseEventType::SCROLL,
      0,
      0,
      QueryCurrentMods(window),
      xoffset,
      yoffset,
      0,
      0,
  });
}
void callback_key(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  ZoneScoped;
  auto instance = InputSystem::instance;
  for (auto f : InputSystem::onCallback) {
    f(window, key, scancode, action, mods);
  }
  instance->currentMods = QueryCurrentMods(window);

  if (action == GLFW_RELEASE) {
    auto it = std::find(instance->heldKeys.begin(), instance->heldKeys.end(),
                        scancode);
    if (it != instance->heldKeys.end()) {
      instance->heldKeys.erase(it);
    }
    auto evIt = std::find_if(instance->events.begin(), instance->events.end(),
                             [scancode](const KeybindEvent &ev) {
                               return ev.scancode == scancode &&
                                      ev.type == KeypressType::HOLD;
                             });
    if (evIt != instance->events.end()) {
      instance->events.erase(evIt);
    }
    if (instance->heldKeys.empty()) {
      instance->currentMods = 0;
    }
  } else if (action == GLFW_PRESS) {
    if (std::find(instance->heldKeys.begin(), instance->heldKeys.end(),
                  scancode) == instance->heldKeys.end()) {
      instance->heldKeys.push_back(scancode);
    }
  }
  instance->events.push_back(
      KeybindEvent{scancode, mods, (KeypressType)action});
}

std::shared_ptr<InputSystem> InputSystem::instance =
    std::make_shared<InputSystem>();

void Engine::InputSystem::Init() {
  ZoneScoped;
  instance = std::make_shared<InputSystem>();
  std::string jsstr;
  auto res = FileUtil::ReadFile(Config::inst->defaults->InputPath, &jsstr);
  auto window = Engine::Graphics::Main::instance->window;
  glfwSetKeyCallback(window, callback_key);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetScrollCallback(window, scroll_callback);
  if (res != 0) {
    return;
  }
  nlohmann::json js;
  try {
    js = nlohmann::json::parse(jsstr);
  } catch (const std::exception &e) {
    return;
  }
  InputSystemJson ijs = js;
  for (auto j : ijs.keybindsPress) {
    auto k = NewKeybind(j.name, j.default_key, j.key, j.mods,
                        KeypressType::PRESS, nullptr, true);
  }
  for (auto j : ijs.keybindsHold) {
    auto k = NewKeybind(j.name, j.default_key, j.key, j.mods,
                        KeypressType::HOLD, nullptr, true);
  }
  for (auto j : ijs.keybindsHoldText) {
    auto k = NewKeybind(j.name, j.default_key, j.key, j.mods,
                        KeypressType::HOLD_TEXT, nullptr, true);
  }
  for (auto j : ijs.keybindsRelease) {
    auto k = NewKeybind(j.name, j.default_key, j.key, j.mods,
                        KeypressType::RELEASE, nullptr, true);
  }
}

void Engine::InputSystem::Save() {
  InputSystemJson js = {};
  for (auto const &[sc, jvec] : instance->keybind_map_press) {
    for (auto const &j : jvec) {
      js.keybindsPress.push_back(
          KeybindJson{j->name, j->default_key, j->key, j->scancode, j->mods});
    }
  }
  for (auto const &[sc, jvec] : instance->keybind_map_hold) {
    for (auto const &j : jvec) {
      js.keybindsHold.push_back(
          KeybindJson{j->name, j->default_key, j->key, j->scancode, j->mods});
    }
  }
  for (auto const &[sc, jvec] : instance->keybind_map_hold_text) {
    for (auto const &j : jvec) {
      js.keybindsHoldText.push_back(
          KeybindJson{j->name, j->default_key, j->key, j->scancode, j->mods});
    }
  }
  for (auto const &[sc, jvec] : instance->keybind_map_release) {
    for (auto const &j : jvec) {
      js.keybindsRelease.push_back(
          KeybindJson{j->name, j->default_key, j->key, j->scancode, j->mods});
    }
  }
  nlohmann::json j = js;
  std::string s = j.dump(2);
  if (FileUtil::SaveFile(Config::inst->defaults->InputPath, &s) != 0) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Failed to save input map to {}",
                        Config::inst->defaults->InputPath);
  }
}

void Engine::InputSystem::ProcessEvents() {
  for (auto event : instance->events) {
    std::map<int, std::vector<std::shared_ptr<Keybind>>> *m = nullptr;
    switch (event.type) {
    case PRESS: {
      m = &instance->keybind_map_press;
      break;
    }
    case HOLD: {
      m = &instance->keybind_map_hold;
      break;
    }
    case HOLD_TEXT: {
      m = &instance->keybind_map_hold_text;
      break;
    }
    case RELEASE: {
      m = &instance->keybind_map_release;
      break;
    }
    case INVALID: {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "INVALID event for {}",
                          event.scancode);
      return;
    }
    }
    if ((*m).contains(event.scancode)) {
      for (auto kb : (*m)[event.scancode]) {
        int effectiveMods = SanitizeMods(event.mods);
        effectiveMods &= ~GetKeyModifierBit(kb->key);
        if (event.type == RELEASE) {
          if (kb->isHeld || effectiveMods == SanitizeMods(kb->mods)) {
            kb->Trigger();
            kb->isHeld = false;
          }
        } else {
          if (effectiveMods == SanitizeMods(kb->mods)) {
            kb->Trigger();
          } else if (event.type == HOLD) {
            kb->isHeld = false;
          }
        }
      }
    }
    if (event.type == RELEASE) {
      if (instance->keybind_map_press.contains(event.scancode)) {
        for (auto kb : instance->keybind_map_press[event.scancode]) {
          kb->isHeld = false;
        }
      }
      if (instance->keybind_map_hold.contains(event.scancode)) {
        for (auto kb : instance->keybind_map_hold[event.scancode]) {
          kb->isHeld = false;
        }
      }
    }
  }
  instance->events.clear();
  for (int scode : instance->heldKeys) {
    instance->events.push_back(
        KeybindEvent{scode, instance->currentMods, KeypressType::HOLD});
  }

  /// Mouse
  for (auto e : instance->keybind_mouse_events) {
    instance->mouseXScrollOffset += e.xscrolloffset;
    instance->mouseYScrollOffset += e.yscrolloffset;
    std::vector<std::shared_ptr<KeybindMouse>> *vec = nullptr;
    switch (e.type) {
    case MOVE_CLICKED:
    case MOVE: {
      vec = &(instance->keybind_vector_mouse_move);
      break;
    }
    case CLICK: {
      if (e.action == GLFW_PRESS) {
        instance->held_mouse_buttons.push_back(e.mbutton);
      } else {
        auto p = std::find(instance->held_mouse_buttons.begin(),
                           instance->held_mouse_buttons.end(), e.mbutton);
        if (p != instance->held_mouse_buttons.end()) {
          instance->held_mouse_buttons.erase(p);
        }
      }
      vec = &(instance->keybind_vector_mouse_move);
      break;
    }
    case SCROLL: {
      vec = &(instance->keybind_vector_mouse_scroll);
      break;
    }
    }
    for (auto &kb : *vec) {
      kb->CallFunc(e.xpos, e.ypos, e.mbutton, e.action, e.mods, e.xscrolloffset,
                   e.yscrolloffset, instance->mouseXScrollOffset,
                   instance->mouseYScrollOffset);
    }
  }
}

std::shared_ptr<Keybind>
Engine::InputSystem::NewKeybind(std::string id, int default_key, int key,
                                int mods, KeypressType type,
                                std::function<void()> func, bool isInit) {
  bool isnew = false;
  std::shared_ptr<Keybind> k;
  if (instance->keybinds.contains(id)) {
    if (instance->keybinds[id]->f != nullptr)
      return GetKeybind(id);
    k = GetKeybind(id);
    k->f = func;
  } else {
    k = std::make_shared<Keybind>(id, default_key, key, mods, type, func);
    instance->keybinds[id] = k;
    isnew = true;
  }

  std::map<int, std::vector<std::shared_ptr<Keybind>>> *m = nullptr;
  switch (type) {
  case PRESS:
    m = &instance->keybind_map_press;
    break;
  case HOLD:
    m = &instance->keybind_map_hold;
    break;
  case HOLD_TEXT:
    m = &instance->keybind_map_hold_text;
    break;
  case RELEASE:
    m = &instance->keybind_map_release;
    break;
  case INVALID:
    m = nullptr;
    break;
  }
  if (m == nullptr) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Map ptr is nullptr! type is {}",
                        (int)type);
    return nullptr;
  }
  auto &vec = (*m)[glfwGetKeyScancode(k->key)];
  if (std::find(vec.begin(), vec.end(), k) == vec.end()) {
    vec.push_back(k);
  }
  SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "New keybind with id {}", id);
  if (isnew) {
    SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Keybind {} is completly new", id);
    Save();
  }
  return k;
}
std::shared_ptr<Keybind> Engine::InputSystem::ChangeKeybind(std::string id,
                                                            int old_key,
                                                            int new_key,
                                                            int mods) {
  auto kb = GetKeybind(id);
  if (kb == nullptr) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                        "Tried to edit a non existing keybind");
    return nullptr;
  }
  std::map<int, std::vector<std::shared_ptr<Keybind>>> *mp = nullptr;
  switch (kb->type) {

  case PRESS:
    mp = &instance->keybind_map_press;
    break;
  case HOLD:
    mp = &instance->keybind_map_hold;
    break;
  case HOLD_TEXT:
    mp = &instance->keybind_map_hold_text;
    break;
  case RELEASE:
    mp = &instance->keybind_map_release;
    break;
  case INVALID:
    mp = nullptr;
    break;
  }
  (*mp)[glfwGetKeyScancode(old_key)].erase(
      std::find((*mp)[glfwGetKeyScancode(old_key)].begin(),
                (*mp)[glfwGetKeyScancode(old_key)].end(), kb));
  (*mp)[glfwGetKeyScancode(new_key)].insert(
      (*mp)[glfwGetKeyScancode(new_key)].end(), kb);
  kb->key = new_key;
  kb->scancode = glfwGetKeyScancode(new_key);
  kb->mods = mods;
  Save();
  return kb;
}

std::shared_ptr<Keybind> Engine::InputSystem::GetKeybind(std::string id) {
  if (instance->keybinds.contains(id))
    return instance->keybinds[id];
  return nullptr;
}

Engine::Keybind::Keybind(std::string name, int default_key, int key, int mods,
                         KeypressType type, std::function<void()> func) {
  this->name = name;
  this->default_key = default_key;
  this->key = key;
  this->isEnabled = false;
  this->isHeld = false;
  this->f = func;
  this->mods = mods;
  this->scancode = glfwGetKeyScancode(key);
  this->type = type;
}

void Engine::Keybind::Trigger() {
  if (isEnabled) {
    isHeld = true;
    if (f != nullptr)
      f();
  }
}

json Engine::Keybind::ToJson() {
  KeybindJson js = {};
  js.default_key = this->default_key;
  js.scancode = glfwGetKeyScancode(this->key);
  js.name = this->name;
  js.key = this->key;
  js.mods = this->mods;
  return js;
}

void Engine::Keybind::FromJson(json &js) {
  KeybindJson kjs = js;
  this->default_key = kjs.default_key;
  this->key = kjs.key;
  this->scancode = kjs.scancode;
  this->mods = kjs.mods;
}
void Engine::KeybindMouseMove::CallFunc(double xposmouse, double yposmouse,
                                        int mbutton, int maction, int mmods,
                                        double xscrolloffset,
                                        double yscrolloffset, double positionX,
                                        double positionY) {
  if (func == nullptr || !isEnabled)
    return;
  if (mmods != this->mmods)
    return;
  if (mtype != NORMAL) {
    auto hmb = &InputSystem::instance->held_mouse_buttons;
    if (std::find(hmb->begin(), hmb->end(), mbutton) != hmb->end()) {
      func(xposmouse, yposmouse);
    }
  } else {
    func(xposmouse, yposmouse);
  }
}
