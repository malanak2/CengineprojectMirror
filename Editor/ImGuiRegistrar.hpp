#pragma once
#include "Engine.hpp"
#include "ImGuiRenderers.hpp"
#include <imgui.h>
#include <implot.h>
#include <tracy/Tracy.hpp>
namespace Editor {
namespace ImGuiR {
inline void Register() {
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
          Editor::ImGuiR::ImGuiRenderer::RenderSceneView(
              Engine::Engine::instance->current_scene);
          Editor::ImGuiR::ImGuiRenderer::RenderObjectInspector();
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
}
} // namespace ImGuiR
} // namespace Editor
