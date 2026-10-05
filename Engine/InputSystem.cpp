#include "InputSystem.hpp"
#include "Config/Config.hpp"
#include "Graphics/Graphics.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include <GLFW/glfw3.h>
#include <exception>
#include <memory>
#include <tracy/Tracy.hpp>

using namespace Engine;
void callback_key(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  auto instance = InputSystem::instance;
  if (action == GLFW_RELEASE) {
    // Assume it had to have been pressed for it to be released - in the other
    // case it crashed the program, buut whatever
    instance->heldKeys.erase(std::find(instance->heldKeys.begin(),
                                       instance->heldKeys.end(), scancode));
    instance->events.erase(
        std::find(instance->events.begin(), instance->events.end(),
                  KeybindEvent{scancode, KeypressType::HOLD}));
  } else if (action == GLFW_PRESS) {
    instance->heldKeys.insert(instance->heldKeys.end(), scancode);
  }
  instance->events.insert(instance->events.end(),
                          KeybindEvent{scancode, (KeypressType)action});
}
std::shared_ptr<InputSystem> InputSystem::instance =
    std::make_shared<InputSystem>();

void Engine::InputSystem::Init() {
  ZoneScoped;
  instance = std::make_shared<InputSystem>();
  std::string jsstr;
  auto res = FileUtil::ReadFile(Config::inst->defaults->InputPath, &jsstr);

  glfwSetKeyCallback(Engine::Graphics::Main::instance->window, callback_key);
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
    auto k =
        NewKeybind(j.name, j.default_key, j.key, KeypressType::PRESS, nullptr);
  }
  for (auto j : ijs.keybindsHold) {
    auto k =
        NewKeybind(j.name, j.default_key, j.key, KeypressType::HOLD, nullptr);
  }
  for (auto j : ijs.keybindsHoldText) {
    auto k = NewKeybind(j.name, j.default_key, j.key, KeypressType::HOLD_TEXT,
                        nullptr);
  }
  for (auto j : ijs.keybindsRelease) {
    auto k = NewKeybind(j.name, j.default_key, j.key, KeypressType::RELEASE,
                        nullptr);
  }
}
void Engine::InputSystem::Save() {
  InputSystemJson js = {};
  for (auto const &[sc, j] : instance->keybind_map_press) {
    js.keybindsPress.insert(
        js.keybindsPress.end(),
        KeybindJson{j->name, j->default_key, j->key, j->scancode});
  }
  for (auto const &[sc, j] : instance->keybind_map_hold) {
    js.keybindsHold.insert(
        js.keybindsHold.end(),
        KeybindJson{j->name, j->default_key, j->key, j->scancode});
  }
  for (auto const &[sc, j] : instance->keybind_map_hold_text) {
    js.keybindsHoldText.insert(
        js.keybindsHoldText.end(),
        KeybindJson{j->name, j->default_key, j->key, j->scancode});
  }
  for (auto const &[sc, j] : instance->keybind_map_release) {
    js.keybindsRelease.insert(
        js.keybindsRelease.end(),
        KeybindJson{j->name, j->default_key, j->key, j->scancode});
  }
}

void Engine::InputSystem::ProcessEvents() {
  for (auto event : instance->events) {
    std::map<int, std::shared_ptr<Keybind>> *m = nullptr;
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
    }
    if ((*m).contains(event.scancode)) {
      auto f = (*m)[event.scancode]->f;
      if (f != nullptr)
        f();
    }
  }
  instance->events.clear();
  for (int scode : instance->heldKeys) {
    instance->events.insert(instance->events.end(),
                            KeybindEvent{scode, KeypressType::HOLD});
  }
}

std::shared_ptr<Keybind>
Engine::InputSystem::NewKeybind(std::string id, int default_key, int key,
                                KeypressType type, std::function<void()> func) {
  auto k = std::make_shared<Keybind>(id, default_key, key, type, func);
  switch (type) {
  case PRESS:
    instance->keybind_map_press[glfwGetKeyScancode(key)] = k;
    break;
  case HOLD:
    instance->keybind_map_hold[glfwGetKeyScancode(key)] = k;
    break;
  case HOLD_TEXT:
    instance->keybind_map_hold_text[glfwGetKeyScancode(key)] = k;
    break;
  case RELEASE:
    instance->keybind_map_release[glfwGetKeyScancode(key)] = k;
    break;
  }
  SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "New keybind with id {}", id);
  return k;
}
Engine::Keybind::Keybind(std::string name, int default_key, int key,
                         KeypressType type, std::function<void()> func) {
  this->name = name;
  this->default_key = key;
  this->key = key;
  this->isEnabled = false;
  this->f = func;
  this->scancode = glfwGetKeyScancode(key);
}
void Engine::Keybind::Trigger() {
  if (isEnabled) {
    if (f)
      f();
  }
}
json Engine::Keybind::ToJson() {
  KeybindJson js = {};
  js.default_key = this->default_key;
  js.scancode = glfwGetKeyScancode(this->key);
  js.name = this->name;
  return js;
}
void Engine::Keybind::FromJson(json &js) {
  KeybindJson kjs = js;
  this->default_key = kjs.default_key;
  this->key = kjs.key;
  this->scancode = kjs.scancode;
}
