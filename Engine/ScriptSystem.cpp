#include "ScriptSystem.hpp"
#include "Components/InvalidComponent.hpp"
#include "Util/LoggerUtil.hpp"
#include <memory>
namespace Engine::Main {
std::shared_ptr<ScriptSystem> ScriptSystem::instance =
    std::make_shared<ScriptSystem>();
} // namespace Engine::Main
