//
// Created by malan on 20.04.2026.
//

#include "Graphics.hpp"
#include "Material.hpp"
#include "Util/LoggerUtil.hpp"
#include <memory>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include "Scene.hpp"

#ifdef IMGUI
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#define IMGUI_SCALE 1
#endif

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

namespace Engine {
    namespace Graphics {
        // Initialize vecotrs
        std::unordered_map<std::string, std::shared_ptr<Shader> > Main::vertexShaders =
                {};
        std::unordered_map<std::string, std::shared_ptr<Shader> > Main::fragmentShaders =
                {};
        std::unordered_map<std::string, std::shared_ptr<Material> > Main::materials = {};

        int Main::Init(Config *config) {
            auto logger = ENGINE_UTIL_LOGGER;
            glfwInit();
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            window =
                    glfwCreateWindow(800, 600, config->window->title.c_str(), NULL, NULL);
            if (window == NULL) {
                SPDLOG_LOGGER_ERROR(spdlog::get("console"),
                                    "Failed to create GLFW window.");
                glfwTerminate();
                return -1;
            }
            glfwMakeContextCurrent(window);
            glfwSwapInterval(1);

#ifdef IMGUI
            SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "IMGUI initializing");
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO &io = ImGui::GetIO();
            (void) io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

            ImGui::StyleColorsDark();

            ImGuiStyle &style = ImGui::GetStyle();
            style.ScaleAllSizes(IMGUI_SCALE);
            style.FontScaleDpi = IMGUI_SCALE;
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                style.WindowRounding = 0.0f;
                style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            }
            ImGui_ImplGlfw_InitForOpenGL(window, true);
            ImGui_ImplOpenGL3_Init("#version 460");
#endif

            if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
                SPDLOG_LOGGER_ERROR(spdlog::get("console"), "Failed to initialize GLAD.");
                return -1;
            }
            glViewport(0, 0, 800, 600);
            glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
            CHECK_GL_ERROR();
            // TODO: Remove
            return 0;
        }

#ifdef IMGUI
        void Main::ShowSceneObjectMenu(std::vector<std::shared_ptr<SceneObject>> *sceneObjects) {
            if (!sceneObjects) return;
            for (auto& obj: *sceneObjects) {
                if (!obj || !obj->instance) continue;
                ImGui::PushID(obj.get());

                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap;
                if (obj->Children.empty()) {
                    flags |= ImGuiTreeNodeFlags_Leaf;
                }

                bool open = ImGui::TreeNodeEx(obj->instance->_name.c_str(), flags);

                if (ImGui::BeginDragDropSource()) {
                    SceneObject* ptr = obj.get();
                    ImGui::SetDragDropPayload("OBJ_PARENT", &ptr, sizeof(SceneObject*));
                    ImGui::Text("%s", obj->instance->_name.c_str());
                    ImGui::EndDragDropSource();
                }

                /*if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OBJ_PARENT")) {
                        SceneObject* draggedObjRaw = *(SceneObject**)payload->Data;
                        std::shared_ptr<SceneObject> draggedObj = draggedObjRaw->shared_from_this();
                        if (draggedObj != obj) {
                            draggedObj->Parent = obj;
                            // Note: Real re-parenting should also update Children vectors
                        }
                    }
                    ImGui::EndDragDropTarget();
                }*/

                if (this->sceneObject != obj) {
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - 50);
                if ( ImGui::Button("Select")) {
                    this->sceneObject = obj;
                }}

                if (open) {
                    ShowSceneObjectMenu(&obj->Children);
                    ImGui::TreePop();
                }

                ImGui::PopID();
            }
        }
#endif

        int Main::Tick(std::shared_ptr<Scene>  scene) {
            if (glfwWindowShouldClose(window)) {
                SPDLOG_LOGGER_INFO(spdlog::get("console"), "GLFW Window should close.");
                return -1;
            }
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            CHECK_GL_ERROR();
#ifdef IMGUI
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking |
                                            ImGuiWindowFlags_NoCollapse |
                                            ImGuiWindowFlags_MenuBar |
                                            ImGuiWindowFlags_DockNodeHost;
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
            // DEMO:
            // ImGui::ShowDemoWindow();
            {
                ImGui::Begin("Scene");
                ImGui::Text("Edit current scene");
                if (ImGui::CollapsingHeader("Objects")) {
                    ShowSceneObjectMenu(&scene->objects);
                }
                if (sceneObject != nullptr) {
                    ImGui::Text(("Scene object " + sceneObject->instance->_name).c_str());
                    if (ImGui::Button("Deselect")) sceneObject = nullptr; else {
                    ImGui::Text("Parent: %s", sceneObject->Parent ? sceneObject->Parent->instance->_name.c_str() : "nullptr");
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OBJ_PARENT")) {
                            SceneObject* draggedObj = *(SceneObject**)payload->Data;
                            sceneObject->SetParent(draggedObj->shared_from_this());
                        }
                        ImGui::EndDragDropTarget();
                    }}
                } else {
                    ImGui::Text("New Scene Object");
                    ImGui::InputText("Name", namebuf, 64);
                    ImGui::InputText("Material path", matbuf, 64);
                    ImGui::InputFloat3("Position: ", coords);
                    ImGui::InputFloat4("Rotation: ", coords);
                    ImGui::Text("Parent: %s", newObjectParent  ? newObjectParent->instance->_name.c_str() : "nullptr");
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OBJ_PARENT")) {
                            SceneObject* draggedObj = *(SceneObject**)payload->Data;
                            newObjectParent= draggedObj->shared_from_this();
                        }
                        ImGui::EndDragDropTarget();
                    }
                    if (newObjectParent != nullptr) {
                        ImGui::SameLine();
                        if (ImGui::Button("Remove")) {
                            newObjectParent= nullptr;
                        }
                    }
                    if (ImGui::Button("Create")) {
                        auto o = std::make_shared<Object>();
                        std::vector<float> pos = {};
                        std::vector<float> rot = {};
                        pos.assign(coords, coords + sizeof(coords)/ sizeof(float));
                        rot.assign(rotation, rotation + sizeof(rotation)/ sizeof(float));
                        o->fromParams(namebuf, {}, pos, rot);
                        scene->Instantiate(o, newObjectParent);
                    }
                }
                ImGui::End();
            }
#endif
            /*
            material->SetupMaterial();
            material->program->SetUniform("color", 1, 0, 0, 1);
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            CHECK_GL_ERROR();
            */
            for (auto &[key, val]: materials) {
                val->SetupMaterial();
                val->RenderObjects();
            }
#ifdef IMGUI
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
            glfwSwapBuffers(window);
            glfwPollEvents();

            return 0;
        }

        void Main::Terminate() {
            materials.clear();
            vertexShaders.clear();
            fragmentShaders.clear();
#ifdef IMGUI
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
#endif
            glfwDestroyWindow(window);
            glfwTerminate();
        }
    } // namespace Graphics
} // namespace Engine
