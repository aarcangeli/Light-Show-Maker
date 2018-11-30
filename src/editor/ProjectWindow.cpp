#include <utility>

#include "ProjectWindow.h"

#include "Application.h"
#include <IconsFontAwesome4.h>

using namespace sm;
using namespace sm::editor;
using namespace ImGui;

ProjectWindow::ProjectWindow() : timelineEditor() {}

void ProjectWindow::showFrame() {
    if (viewportWidth <= 0 || viewportHeight <= 0 || !proj) return;

    timelineEditor.dpi = dpi;

    timelineHeight = ((float) viewportHeight) * TIMELINE_HEIGHT_RATIO;
    centerHeight = viewportHeight - timelineHeight - menuHeight - spacing;
    leftPanelWidth = ((float) viewportWidth) * TIMELINE_PANELS_WIDTH_RATIO;
    rightPanelWidth = ((float) viewportWidth) * TIMELINE_PANELS_WIDTH_RATIO;
    menuHeight = GImGui->FontBaseSize + GImGui->Style.FramePadding.y * 2.0f;
    spacing = dpi;

    showMenu();
    leftPanelWindow();
    rightPanelWindow();
    outputWindow();
    timelineWindow();
}

void ProjectWindow::outputWindow() {
    SetNextWindowPos(ImVec2{leftPanelWidth, menuHeight});
    SetNextWindowSize(ImVec2{viewportWidth - leftPanelWidth - rightPanelWidth, centerHeight});
    Begin("Output", nullptr, FLAGS);

    Text("CIAO");

    End();
}

void ProjectWindow::leftPanelWindow() {
    SetNextWindowPos(ImVec2{0, menuHeight});
    SetNextWindowSize(ImVec2{leftPanelWidth - spacing, centerHeight});
    Begin("Left", nullptr, FLAGS);

    Text("CIAO");

    End();
}

void ProjectWindow::rightPanelWindow() {
    SetNextWindowPos(ImVec2{viewportWidth - rightPanelWidth + spacing, menuHeight});
    SetNextWindowSize(ImVec2{rightPanelWidth - spacing, centerHeight});
    Begin("Right", nullptr, FLAGS);

    Text("CIAO");

    End();
}

void ProjectWindow::timelineWindow() {
    SetNextWindowPos(ImVec2{0, viewportHeight - timelineHeight});
    SetNextWindowSize(ImVec2{(float) viewportWidth, timelineHeight});
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    Begin("Timeline", nullptr, FLAGS | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    PopStyleVar(1);

    timelineEditor.editorOf(proj->canvas);

    End();
}

void ProjectWindow::showMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File " ICON_FA_GITHUB_SQUARE)) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                gApp->open(std::make_shared<project::Project>());
            }
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                // todo
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                // todo
            }
            if (ImGui::MenuItem("Close", nullptr, nullptr, bool(proj))) {
                gApp->close();
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
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window")) {
            if (ImGui::MenuItem("Restore default layout")) {
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void ProjectWindow::resize(int width, int height) {
    viewportWidth = width;
    viewportHeight = height;
}

void ProjectWindow::open(std::shared_ptr<project::Project> _proj) {
    proj = std::move(_proj);
}

void ProjectWindow::close() {
    proj.reset();
}
