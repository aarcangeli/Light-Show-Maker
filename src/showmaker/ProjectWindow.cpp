#include <utility>

#include "ProjectWindow.h"

#include "Application.h"
#include <IconsFontAwesome4.h>

using namespace sm;
using namespace ImGui;

ProjectWindow::ProjectWindow(Application *app) : app(app), timelineEditor() {}

void ProjectWindow::showFrame() {
    if (viewportWidth <= 0 || viewportHeight <= 0 || !proj) return;

    timelineEditor.dpi = dpi;

    mainFrame();

    outputWindow();
    timelineWindow();
    restoreDefaultLayout = false;
}

void ProjectWindow::mainFrame() {

    SetNextWindowPos(ImVec2{0, 0});
    SetNextWindowSize(ImVec2{(float) viewportWidth, (float) viewportHeight});
    PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));
    Begin(proj->name.c_str(), nullptr,
          ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
          ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus);
    PopStyleVar(2);
    menuRect = GetCurrentWindow()->MenuBarRect();

    showMenu();

    myEditor.editorOf(proj.get());

    End();
}

void ProjectWindow::outputWindow() {
    int flags = 0;
    if (FIXED_LAYOUT || restoreDefaultLayout) {
        float menuHeight = menuRect.Max.y - menuRect.Min.y + 1;
        float height = ((float) viewportHeight) * (1 - TIMELINE_HEIGHT_RATIO) - menuHeight;
        float width = height * OUTPUT_ASPECT_RATIO;
        SetNextWindowPos(ImVec2{(viewportWidth - width) / 2, menuHeight});
        SetNextWindowSize(ImVec2{width, height});
    }
    if (FIXED_LAYOUT) {
        flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse;
    }
    Begin("Output", nullptr, flags);

    Text("CIAO");

    End();
}

void ProjectWindow::timelineWindow() {
    int flags = 0;
    if (FIXED_LAYOUT || restoreDefaultLayout) {
        float height = ((float) viewportHeight) * TIMELINE_HEIGHT_RATIO;
        SetNextWindowPos(ImVec2{0, viewportHeight - height});
        SetNextWindowSize(ImVec2{(float) viewportWidth, height});
    }
    if (FIXED_LAYOUT) {
        flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    }
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    Begin("Timeline", nullptr, flags);
    PopStyleVar(1);

    timelineEditor.editorOf(proj->canvas);

    End();
}

void ProjectWindow::showMenu() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File " ICON_FA_GITHUB_SQUARE )) {
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
                int theme = app->getTheme();
                if (ImGui::Combo("Theme", &theme, "Classic\0Dark\0Light\0\0")) {
                    app->setTheme(theme);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window")) {
            if (ImGui::MenuItem("Restore default layout")) {
                restoreDefaultLayout = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
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
