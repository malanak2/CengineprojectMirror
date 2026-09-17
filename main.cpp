#include <csignal>
#include <exception>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <stacktrace>
#include <tracy/Tracy.hpp>

#include "Editor/ImGuiRenderers.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/Graphics.hpp"
#include "Graphics/Texture.hpp"
#include "ScriptSystem.hpp"
#include "Util/LoggerUtil.hpp"

void sigsegvHandler(int sig) {
  SPDLOG_LOGGER_CRITICAL(ENGINE_UTIL_LOGGER,
                         "Program has crashed. Stacktrace: {}",
                         std::to_string(std::stacktrace::current()));
  exit(sig);
}

void sigabrtHandler(int sig) { sigsegvHandler(sig); }

int main() {
  signal(SIGSEGV, sigsegvHandler);
  signal(SIGABRT, sigabrtHandler);
  try {
    /// Inject functions
    /// Init engine
    Engine::Graphics::Main::instance->init->insert(
        Engine::Graphics::Main::instance->init->end(), []() {
          SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "IMGUI initializing");
          IMGUI_CHECKVERSION();
          ImGui::CreateContext();
          ImPlot::CreateContext();
          ImGuiIO &io = ImGui::GetIO();
          (void)io;
          io.ConfigFlags |=
              ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

          ImGui::StyleColorsDark();

          ImGuiStyle &style = ImGui::GetStyle();
          style.ScaleAllSizes(IMGUI_SCALE);
          style.FontScaleDpi = IMGUI_SCALE;
          if (io.ConfigFlags) { // ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
          }
          ImGui_ImplGlfw_InitForOpenGL(Engine::Graphics::Main::instance->window,
                                       true);
          ImGui_ImplOpenGL3_Init("#version 460");
        });
    Engine::Graphics::Main::instance->preRender->insert(
        Engine::Graphics::Main::instance->preRender->end(), []() {
          ImGui_ImplOpenGL3_NewFrame();
          ImGui_ImplGlfw_NewFrame();
          ImGui::NewFrame();

          ImGuiWindowFlags window_flags =
              ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar;
          //           ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
          //           ImGuiDockNodeFlags_PassthruCentralNode);
          // DEMO:
          //         ImGui::ShowDemoWindow();
          //       ImPlot::ShowDemoWindow();
          {
            ZoneScopedN("ImGui");
            Editor::ImGuiRenderers::ImGuiRenderer::RenderSceneView(
                Engine::Engine::instance->current_scene);
            Editor::ImGuiRenderers::ImGuiRenderer::RenderObjectInspector();
          }
        });
    Engine::Graphics::Main::instance->postRender->insert(
        Engine::Graphics::Main::instance->postRender->end(), []() {
          if (Config::inst->graphics->enableAntiAliasing) {
            glDisable(GL_MULTISAMPLE);
          }
          ImGui::Render();
          ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
          if (Config::inst->graphics->enableAntiAliasing) {
            glEnable(GL_MULTISAMPLE);
          }
        });

    Engine::Graphics::Main::instance->terminate->insert(
        Engine::Graphics::Main::instance->terminate->end(), []() {
          ImGui_ImplOpenGL3_Shutdown();
          ImGui_ImplGlfw_Shutdown();
          ImPlot::DestroyContext();
          ImGui::DestroyContext();
        });
    Engine::Engine::Init();
    Editor::ImGuiRenderers::ImGuiUniformRenderer::Init();
    /// Register components
    // REGISTER_SCRIPT(script);
    /// Load scene
    Engine::Engine::LoadScene();
    /// Run
    Engine::Engine::instance->Run();
  } catch (const std::exception &e) {
    SPDLOG_LOGGER_CRITICAL(ENGINE_UTIL_LOGGER,
                           "Program has crashed. Stacktrace: {}",
                           std::to_string(std::stacktrace::current()));
  }
  return 0;
}
