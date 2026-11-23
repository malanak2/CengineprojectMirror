#pragma once
#define IMGUI_TEST_ENGINE_ENABLE_COROUTINE_STDTHREAD_IMPL 1
#define IMGUI_TEST_ENGINE_ENABLE_IMPLOT 1

#ifdef UNIT_TESTING
#include "External/imgui_test_engine/imgui_test_engine/imgui_te_context.h"
#include "External/imgui_test_engine/imgui_test_engine/imgui_te_engine.h" // IM_REGISTER_TEST()
#include "External/imgui_test_engine/imgui_test_engine/imgui_te_imconfig.h"
#include "External/imgui_test_engine/imgui_test_engine/imgui_te_ui.h"
#include "External/imgui_test_engine/imgui_test_engine/imgui_te_utils.h" // ImHashDecoratedPath()
#endif
#include "imgui.h"
#include "implot.h"
namespace Editor {
class Testing {
public:
  static void Setup();
  static void Shutdown();

#ifdef UNIT_TESTING
  static ImGuiTestEngine *engine;
#endif
};
} // namespace Editor
