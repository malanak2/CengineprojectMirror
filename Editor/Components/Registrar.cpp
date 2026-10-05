#include "Registrar.hpp"
#include "Interfaces/IIdentifiable.hpp"
#include "ScriptSystem.hpp"
#include "TestInputComponent.hpp"
#include "Util/LoggerUtil.hpp"
void Editor::Registrar::RegisterComponents() {
  SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Registering engine scripts...");
  REGISTER_SCRIPT(TestInputComponent);
}
