#ifndef OUTPUTVIDEOEDITOR_H
#define OUTPUTVIDEOEDITOR_H

#include "Project.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "DecorationDragger.h"
#include "Decoration.h"

namespace sm {
namespace editor {

class OutputVideoEditor {
    const float RATIO = 16.f / 9;
    const float TOPBAR_HEIGHT = 40;
    const float CONTENT_PADDING = 10;
    const ImVec2 logicalSize = {1920, 1080};

    const ImU32 COLOR_BORDER = IM_COL32(127, 127, 127, 255 * 0.4);

public:
    OutputVideoEditor() : dragger(this) {};

    void editorOf(std::shared_ptr<project::Project> proj);

    ImRect getDecorationRegion(const std::shared_ptr<project::Decoration> &dec);
    ImVec2 getLogicalScale() const;
    bool isFocused() const { return windowFocused; };

    void deleteDecoration(const std::shared_ptr <project::Decoration> &dec);

private:
    void openImage(const std::shared_ptr<project::Project> &proj) const;
    void topMenu(const std::shared_ptr<project::Project> &proj);
    void drawContent(std::shared_ptr<project::Project> ptr);
    void drawCanvas(project::Canvas &canvas);
    void printDecoration(const std::shared_ptr<project::Decoration> &shared_ptr);

    ImVec2 canvasScreenPos;
    ImVec2 canvasSize;

    DecorationDragger dragger;
    bool mouseClicked = false;
    bool windowFocused = false;
    ImVec2 mousePos;
    std::shared_ptr<project::Decoration> decorationHover;
    std::shared_ptr<project::Decoration> lastDecorationHover;
    std::shared_ptr<project::Decoration> decorationToDelete;

    void append(const std::shared_ptr<project::Project> &proj, const std::shared_ptr<project::Decoration> &dec) const;
};

}
}

#endif //OUTPUTVIDEOEDITOR_H
