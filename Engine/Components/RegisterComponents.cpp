#include "RegisterComponents.hpp"
#include "Components/TestComponent.hpp"
#include "Graphics/Components/CameraComponent.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
void Engine::ScriptRegistrar::RegisterCommonComponents() {
  REGISTER_SCRIPT(TestComponent);
}
void Engine::ScriptRegistrar::RegisterGraphicComponents() {
  REGISTER_SCRIPT(Graphics::ComponentRenderable);
  REGISTER_SCRIPT(Graphics::CameraComponent);
}
void Engine::ScriptRegistrar::RegisterComponents() {
  RegisterCommonComponents();
  RegisterGraphicComponents();
};
;
;
