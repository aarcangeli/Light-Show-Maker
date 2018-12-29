#include <utility>

#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include "core.h"
#include "Editor.h"
#include "memory"
#include <imgui.h>
#include <imgui_internal.h>
#include "TimelineEditor.h"
#include "OutputVideoEditor.h"
#include "PropertyPanel.h"

namespace sm {
namespace editor {

class ProjectWindow {
    const char *MODAL_ERROR = "Error!";
    const float TIMELINE_HEIGHT_RATIO = 0.7;
    const float TIMELINE_PANELS_WIDTH_RATIO = 0.15;

    const int FLAGS = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse;

public:
    explicit ProjectWindow();

    // invoked by Application
    void open(const std::shared_ptr<model::Project> &shared_ptr);
    void close();

    void showError(std::string errorMsg) {
        lastError = std::move(errorMsg);
        openErrorBox = true;
    }

    void resize(int width, int height);

    void showFrame();

private:
    std::shared_ptr<model::Project> proj;
    Editor myEditor;
    TimelineEditor timelineEditor;
    OutputVideoEditor outputPreview;
    PropertyPanel propertyPanel;
    int viewportWidth = 0, viewportHeight = 0;
    bool openErrorBox = false;
    std::string lastError;
    bool maximizeOutput = false;

    void errorBox();

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

    void saveAs() const;
};

}
}

#endif //PROJECTWINDOW_H
