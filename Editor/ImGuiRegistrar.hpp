#pragma once
#include "Engine.hpp"
#include "ImGui/Window.hpp"
#include "ImGui/WindowInputMaps.hpp"
#include "ImGui/WindowObjectInspector.hpp"
#include "ImGui/WindowScene.hpp"
#include "ImGui/WindowToggles.hpp"
#include "ImGuiRenderers.hpp"
#include <imgui.h>
#include <implot.h>
#include <memory>
#include <tracy/Tracy.hpp>
namespace Editor {
namespace ImGuiR {
class ImGuiRegistrar {
public:
  std::vector<std::shared_ptr<Window>> windows = {};
  std::shared_ptr<WindowToggles> wtgles;

public:
  void Register() {
    windows.insert(windows.end(), std::make_shared<WindowScene>());
    windows.insert(windows.end(),
                   std::make_shared<WindowObjectInspector>(
                       std::static_pointer_cast<WindowScene>(windows[0])));
    windows.insert(windows.end(), std::make_shared<WindowInputMaps>());
    wtgles = std::make_shared<WindowToggles>();
    wtgles->windows = &windows;
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
        Engine::Graphics::Main::instance->preRender->end(), [this]() {
          {
            ZoneScopedN("ImGui Setup");
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGuiWindowFlags window_flags =
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar;
          }
          //           ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
          //           ImGuiDockNodeFlags_PassthruCentralNode);
          {
            ZoneScopedN("ImGui build up frame");
            for (auto w : windows) {
              w->Draw();
            }
            wtgles->Draw();
          }
        });
    Engine::Graphics::Main::instance->postRender->insert(
        Engine::Graphics::Main::instance->postRender->end(), []() {
          ZoneScopedN("ImGui Render");
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
  }
};
} // namespace ImGuiR
} // namespace Editor
