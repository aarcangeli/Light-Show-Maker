#ifndef OUTPUTVIDEOEDITOR_H
#define OUTPUTVIDEOEDITOR_H

#include "Project.h"
#include "imgui.h"

namespace sm {
namespace editor {

class OutputVideoEditor {
    const float RATIO = 16.f / 9;
    const float TOPBAR_HEIGHT = 40;
    const float CONTENT_PADDING = 10;
    const ImVec2 logicalSize = {1920, 1080};

    const ImU32 COLOR_BORDER = IM_COL32(127, 127, 127, 255 * 0.4);

public:
    void editorOf(std::shared_ptr<project::Project> proj);

private:
    void openImage(const std::shared_ptr<project::Project> &proj) const;
    void topMenu(const std::shared_ptr<project::Project> &proj) const;
    void drawContent(std::shared_ptr<project::Project> ptr);
    void drawCanvas(const project::Canvas &canvas);
    void printDecoration(const std::shared_ptr<project::Decoration> &shared_ptr);

    ImVec2 canvasScreenPos;
    ImVec2 canvasSize;

};

}
}

#endif //OUTPUTVIDEOEDITOR_H
