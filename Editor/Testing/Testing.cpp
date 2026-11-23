#include "Testing.hpp"

#include "Engine.hpp"
#include "TestingComponent.hpp"

#ifdef UNIT_TESTING
#include "External/imgui_test_engine/imgui_test_engine/imgui_te_engine.h"
#include "External/imgui_test_engine/imgui_test_engine/imgui_te_utils.h"
#endif
#include "Util/LoggerUtil.hpp"
#ifdef UNIT_TESTING
ImGuiTestEngine *Editor::Testing::engine = nullptr;
#endif
void Editor::Testing::Setup() {
#ifdef UNIT_TESTING
  ZoneScoped;
  SPDLOG_LOGGER_DEBUG(ENGINE_UTIL_LOGGER, "Setting up testing...");
  engine = ImGuiTestEngine_CreateContext();
  ImGuiTestEngineIO &test_io = ImGuiTestEngine_GetIO(engine);
  test_io.ConfigVerboseLevel = ImGuiTestVerboseLevel_Info;
  test_io.ConfigVerboseLevelOnError = ImGuiTestVerboseLevel_Debug;
  // Important: Tests need to run in-order
  auto t = IM_REGISTER_TEST(engine, "tests-objects", "test-create-objet");
  t->TestFunc = [](ImGuiTestContext *ctx) {
    ctx->SetRef("Scene");
    ctx->ItemClick("Objects", 1);
    ctx->ItemClick("**/New object");
    IM_CHECK_NE(
        Engine::Engine::instance->current_scene->FindObject("New Object"),
        nullptr);
  };
  t = IM_REGISTER_TEST(engine, "tests-objects", "test-rename-object");
  t->TestFunc = [](ImGuiTestContext *ctx) {
    ctx->SetRef("Scene");
    ctx->ItemOpen("Objects");
    ctx->ItemClick("**/Select##New Object");
    ctx->SetRef("Object Inspector");
    ctx->ItemInputValue("Name:", "Rename test");
    IM_CHECK_NE(
        Engine::Engine::instance->current_scene->FindObject("Rename test"),
        nullptr);
    IM_CHECK_EQ(
        Engine::Engine::instance->current_scene->FindObject("New Object"),
        nullptr);
  };

  t = IM_REGISTER_TEST(engine, "tests-objects", "test-add-component");
  t->TestFunc = [](ImGuiTestContext *ctx) {
    ctx->SetRef("Object Inspector");
    auto object =
        Engine::Engine::instance->current_scene->FindObject("Rename test");
    ctx->ItemClick("TestingComponent");
    IM_CHECK_NE(object->GetComponent<TestingComponent>(), nullptr);
  };

  t = IM_REGISTER_TEST(engine, "tests-objects", "test-component-fields");
  t->TestFunc = [](ImGuiTestContext *ctx) {
    ctx->SetRef("Object Inspector");
    auto object =
        Engine::Engine::instance->current_scene->FindObject("Rename test");
    ctx->ItemOpen("TestingComponent");
    auto comp = object->GetComponent<TestingComponent>();
    ctx->ItemInputValue("**/testInt", 69);
    IM_CHECK_EQ(comp->testInt, 69);
    ctx->ItemInputValue("**/testString", "test");
    IM_CHECK_STR_EQ(comp->testString.c_str(), "test");
  };

  t = IM_REGISTER_TEST(engine, "tests-objects", "test-remove-component");
  t->TestFunc = [](ImGuiTestContext *ctx) {
    ctx->SetRef("Object Inspector");
    auto object =
        Engine::Engine::instance->current_scene->FindObject("Rename test");
    ctx->ItemOpen("TestingComponent");
    ctx->ItemClick("Remove##TestingComponent");
    IM_CHECK_EQ(object->GetComponent<TestingComponent>(), nullptr);
  };

  t = IM_REGISTER_TEST(engine, "tests-objects", "test-remove-object");
  t->TestFunc = [](ImGuiTestContext *ctx) {
    ctx->SetRef("Scene");
    ctx->ItemOpen("Objects");
    ctx->ItemClick("**/Rename test", 1);
    ctx->ItemClick("**/Delete##Rename test");
    IM_CHECK_EQ(
        Engine::Engine::instance->current_scene->FindObject("Rename test"),
        nullptr);
  };

  // Start test engine
  ImGuiTestEngine_Start(engine, ImGui::GetCurrentContext());

#endif
}

void Editor::Testing::Shutdown() {
#ifdef UNIT_TESTING
  ImGuiTestEngine_Stop(engine);

  // We shutdown the Dear ImGui context _before_ the test engine context, so
  // .ini data may be saved.
  ImGui::DestroyContext();
  ImGuiTestEngine_DestroyContext(engine);
#endif
}