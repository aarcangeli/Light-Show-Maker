#define IMGUI_DEFINE_MATH_OPERATORS

#include <utility>
#include "ProjectWindow.h"
#include "Application.h"
#include <IconsFontAwesome4.h>
#include <fstream>

using namespace sm;
using namespace sm::editor;
using namespace ImGui;

ProjectWindow::ProjectWindow() : timelineEditor() {}

void ProjectWindow::showFrame() {
    if (viewportWidth <= 0 || viewportHeight <= 0) return;

    timelineEditor.dpi = dpi;

    timelineHeight = ((float) viewportHeight) * TIMELINE_HEIGHT_RATIO;
    centerHeight = viewportHeight - timelineHeight - menuHeight - spacing;
    leftPanelWidth = ((float) viewportWidth) * TIMELINE_PANELS_WIDTH_RATIO;
    rightPanelWidth = ((float) viewportWidth) * TIMELINE_PANELS_WIDTH_RATIO;
    menuHeight = GImGui->FontBaseSize + GImGui->Style.FramePadding.y * 2.0f;
    spacing = dpi;

    showMenu();

    if (proj) {
        leftPanelWindow();
        rightPanelWindow();
        outputWindow();
        timelineWindow();
    }

    if (openErrorBox) {
        OpenPopup(MODAL_ERROR);
        openErrorBox = false;
    }
    errorBox();
}

void ProjectWindow::outputWindow() {
    if (!proj) return;

    SetNextWindowPos(ImVec2{leftPanelWidth, menuHeight});
    SetNextWindowSize(ImVec2{viewportWidth - leftPanelWidth - rightPanelWidth, centerHeight});
    PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    Begin("Output", nullptr, FLAGS | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    PopStyleVar(1);

    outputPreview.editorOf(proj);

    End();
    PopStyleVar(1);
}

void ProjectWindow::leftPanelWindow() {
    if (!proj) return;

    SetNextWindowPos(ImVec2{0, menuHeight});
    SetNextWindowSize(ImVec2{leftPanelWidth - spacing, centerHeight});
    Begin("Project", nullptr, FLAGS);

    for (auto &c : proj->canvas.groups) {
        if (TreeNode(c->name.c_str())) {
            if (Selectable("test")) {
            }
            TreePop();
        }
    }

    End();
}

void ProjectWindow::rightPanelWindow() {
    SetNextWindowPos(ImVec2{viewportWidth - rightPanelWidth + spacing, menuHeight});
    SetNextWindowSize(ImVec2{rightPanelWidth - spacing, centerHeight});
    Begin("Right", nullptr, FLAGS);

    auto dec = gApp->decorationSelected();
    if (dec.size() == 1) {
        ImGui::SliderFloat("size", &dec[0]->size, 1, 100, "%.2f");
    }

    End();
}

void ProjectWindow::timelineWindow() {
    if (!proj) return;

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
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                gApp->open(std::make_shared<project::Project>());
            }
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                std::string outPath = gApp->getPath("clsproj", false);
                if (!outPath.empty()) {
                    if (gApp->load(outPath)) {
                        gApp->saveLastDirectory(outPath);
                    }
                    gApp->filename = outPath;
                }
            }
            if (ImGui::MenuItem("Save", "Ctrl+S", nullptr, bool(proj))) {
                std::ifstream testOpen(gApp->filename);
                bool exists = !testOpen.fail();
                testOpen.close();
                if (!exists) {
                    gApp->filename = "";
                    saveAs();
                } else {
                    gApp->save(gApp->filename);
                }
            }
            if (ImGui::MenuItem("Save As..", "Ctrl+Shift+S", nullptr, bool(proj))) {
                saveAs();
            }
            if (ImGui::MenuItem("Close", nullptr, nullptr, bool(proj))) {
                gApp->close();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Quit")) {
                gApp->quit();
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

void ProjectWindow::saveAs() const {
    std::__cxx11::string outPath = gApp->getPath("clsproj", true);
    if (!outPath.empty()) {
        if (!endsWith(outPath, ".clsproj")) {
            outPath += ".clsproj";
        }
        if (gApp->save(outPath)) {
            gApp->saveLastDirectory(outPath);
        }
    }
}

void ProjectWindow::resize(int width, int height) {
    viewportWidth = width;
    viewportHeight = height;
}

void ProjectWindow::open(const std::shared_ptr<project::Project> &_proj) {
    proj = _proj;
}

void ProjectWindow::close() {
    proj.reset();
}

void ProjectWindow::errorBox() {
    if (BeginPopupModal(MODAL_ERROR, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        Text("%s", lastError.c_str());
        SetCursorScreenPos(GetCursorScreenPos() + ImVec2((GetContentRegionAvail().x - 120) / 2, 0));
        if (Button("OK", ImVec2(120, 0))) { CloseCurrentPopup(); }
        SetItemDefaultFocus();
        EndPopup();
    }
}
