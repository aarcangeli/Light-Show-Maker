#ifndef OUTPUTVIDEOEDITOR_H
#define OUTPUTVIDEOEDITOR_H

#include "Project.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "DecorationDragger.h"
#include "Decoration.h"
#include "ScrollablePane.h"

namespace sm {
namespace editor {

class OutputVideoEditor {
    const float TOPBAR_HEIGHT = 40;
    const float CONTENT_PADDING = 10;
    const ImVec2 logicalSize = {1920, 1080};
    const ImVec2 OFFSET = {800, 0};

    const ImU32 COLOR_BORDER = IM_COL32(127, 127, 127, 255 * 0.4);

public:
    OutputVideoEditor();

    void init(const std::shared_ptr<model::Project> &proj);

    void showEditor();

    ImRect getDecorationRegion(const std::shared_ptr<model::Decoration> &dec);
    ImVec2 getLogicalScale() const;
    bool isFocused() const { return windowFocused; };

    void deleteDecoration(const std::shared_ptr <model::Decoration> &dec);

private:
    void openImage(const std::shared_ptr<model::Project> &proj) const;
    void topMenu();
    void drawContent();
    void drawCanvas(model::Canvas &canvas);
    void drawVector(float alpha, std::vector<std::shared_ptr<model::Decoration>> &array, bool b);
    void printDecoration(float decAlpha, const std::shared_ptr<model::Decoration> &shared_ptr, bool b);

    std::shared_ptr<model::Project> proj;
    ImVec2 canvasScreenPos;
    ImVec2 canvasSize;

    DecorationDragger dragger;
    bool mouseClicked = false;
    bool windowFocused = false;
    ImVec2 mousePos;
    ScrollablePane scrollablePane;

    std::shared_ptr<model::Decoration> decorationHover;
    std::shared_ptr<model::Decoration> lastDecorationHover;
    std::shared_ptr<model::Decoration> decorationToDelete;

    void append(const std::shared_ptr<model::Project> &proj, const std::shared_ptr<model::Decoration> &dec) const;
};

}
}

#endif //OUTPUTVIDEOEDITOR_H
