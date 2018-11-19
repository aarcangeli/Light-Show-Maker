#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include "Editor.h"
#include "memory"
#include <imgui.h>
#include <imgui_internal.h>
#include "TimelineEditor.h"

namespace sm {
class Application;

class ProjectWindow {
    const float TIMELINE_HEIGHT_RATIO = 0.4;
    const float OUTPUT_ASPECT_RATIO = 16.f / 9;
    const bool FIXED_LAYOUT = true;

public:
    explicit ProjectWindow(Application *app);

    // invoked by Application
    void open(std::shared_ptr<project::Project> shared_ptr);
    void close();

    void resize(int width, int height);

    void showFrame();

private:
    Application *app;
    std::shared_ptr<project::Project> proj;
    editor::Editor myEditor;
    editor::TimelineEditor timelineEditor;
    int viewportWidth = 0, viewportHeight = 0;
    bool restoreDefaultLayout = true;
    ImRect menuRect;

    void showMenu();
    void mainFrame();
    void outputWindow();
    void timelineWindow();
};

}

#endif //PROJECTWINDOW_H
