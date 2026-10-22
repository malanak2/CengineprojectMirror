#include "ComponentTestTrigger.hpp"
#include "Object.hpp"
#include "Physics/Components/ComponentBoundingBox.hpp"
#include "Util/LoggerUtil.hpp"
#include <functional>
void Editor::ComponentTestTrigger::Setup() {
  auto o = object.lock();
  auto bb = o->GetComponent<Engine::Physics::ComponentBoundingBox>();
  bb->OnTriggerEnter =
      [this](std::shared_ptr<Engine::Physics::ComponentBoundingBox> other) {
        if (auto o = other->object.lock()) {
          SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Trigger {} hit by {}",
                             object.lock()->_name, o->_name);
        }
      };
}
void Editor::ComponentTestTrigger::End() {
  auto o = object.lock();
  auto bb = o->GetComponent<Engine::Physics::ComponentBoundingBox>();
  bb->OnTriggerEnter = nullptr;
}
