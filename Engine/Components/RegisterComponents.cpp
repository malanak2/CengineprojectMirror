#include "RegisterComponents.hpp"
#include "Components/TestComponent.hpp"
#include "Graphics/Components/CameraComponent.hpp"
#include "Graphics/Components/ComponentAnimator.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
#include "Graphics/Components/ComponentSocket.hpp"
#include "ScriptSystem.hpp"
void Engine::ScriptRegistrar::RegisterCommonComponents() {
  REGISTER_SCRIPT(TestComponent);
}
void Engine::ScriptRegistrar::RegisterGraphicComponents() {
  REGISTER_SCRIPT(Graphics::ComponentRenderable);
  REGISTER_SCRIPT(Graphics::CameraComponent);
  REGISTER_SCRIPT(Graphics::ComponentAnimator);
  REGISTER_SCRIPT(Graphics::ComponentSocket);
}
void Engine::ScriptRegistrar::RegisterPhysicsComponents() {}

void Engine::ScriptRegistrar::RegisterComponents() {
  RegisterCommonComponents();
  RegisterGraphicComponents();
  RegisterPhysicsComponents();
};
