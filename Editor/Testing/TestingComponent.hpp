#pragma once
#include "ScriptSystem.hpp"
#include "Interfaces/IComponent.hpp"

namespace Editor {
enum TestEnum
{
  TESTONE,
  TESTTWO,
};
class [[=Engine::Attr::Script{}]] TestingComponent :public Engine::IComponent
{
  REGISTER_CLASS(TestingComponent);
public:
  [[=Engine::Attr::Editor{}]] int testInt;
  [[=Engine::Attr::Editor{}]] std::string testString;
  [[=Engine::Attr::Editor{}]][[=Engine::Attr::ReadOnly{}]] std::string testReadOnlyString;
  [[=Engine::Attr::Editor{}]] TestEnum testEnum;
};
}