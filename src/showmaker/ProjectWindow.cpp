#include <utility>

#include "ProjectWindow.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "Application.h"

using namespace sm;
using namespace ImGui;

ProjectWindow::ProjectWindow(Application *app) : app(app) {}

void ProjectWindow::showFrame() {
    if (viewportWidth <= 0 || viewportHeight <= 0 || !proj) return;

    SetNextWindowPos(ImVec2{0, 0});
    SetNextWindowSize(ImVec2{(float) viewportWidth, (float) viewportHeight});
    PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));
    Begin(proj->name.c_str(), nullptr,
          ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
          ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus);
    PopStyleVar(2);

    showMenu();

    myEditor.editorOf(proj.get());

    End();
}

void ProjectWindow::showMenu() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                app->open(std::make_shared<project::Project>());
            }
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                // todo
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                // todo
            }
            if (ImGui::MenuItem("Close", nullptr, nullptr, bool(proj))) {
                app->close();
            }
            if (ImGui::MenuItem("Save As..", "Ctrl+Shift+S")) {
                // todo
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Quit")) {
                //close();
//                if (!proj) {
//                    exit = true;
//                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {
                // todo
            }
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {
                // todo
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {
                // todo
            }
            if (ImGui::MenuItem("Copy", "CTRL+C")) {
                // todo
            }
            if (ImGui::MenuItem("Paste", "CTRL+V")) {
                // todo
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Options")) {
                if (ImGui::Combo("Theme", &app->theme, "Classic\0Dark\0Light\0\0")) app->applyTheme();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void ProjectWindow::open(std::shared_ptr<project::Project> _proj) {
    proj = std::move(_proj);
}

void ProjectWindow::close() {
    proj.reset();
}
