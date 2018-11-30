#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include "core.h"
#include "Editor.h"
#include "memory"
#include <imgui.h>
#include <imgui_internal.h>
#include "TimelineEditor.h"

namespace sm {
namespace editor {

class ProjectWindow {
    const float TIMELINE_HEIGHT_RATIO = 0.4;
    const float TIMELINE_PANELS_WIDTH_RATIO = 0.15;

    const int FLAGS = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse;

public:
    explicit ProjectWindow();

    // invoked by Application
    void open(std::shared_ptr<project::Project> shared_ptr);
    void close();

    void resize(int width, int height);

    void showFrame();

    float dpi;

private:
    std::shared_ptr<project::Project> proj;
    editor::Editor myEditor;
    editor::TimelineEditor timelineEditor;
    int viewportWidth = 0, viewportHeight = 0;

    float menuHeight;
    float centerHeight;
    float timelineHeight;
    float leftPanelWidth;
    float rightPanelWidth;
    float spacing;

    void showMenu();
    void outputWindow();
    void timelineWindow();
    void leftPanelWindow();
    void rightPanelWindow();
};

}
}

#endif //PROJECTWINDOW_H
