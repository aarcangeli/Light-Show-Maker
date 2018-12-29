#ifndef DECORATIONDRAGGER_H
#define DECORATIONDRAGGER_H

#include "Decoration.h"
#include "memory"
#include "imgui.h"
#include "imgui_internal.h"

namespace sm {
namespace editor {

class OutputVideoEditor;

class DecorationDragger {
    const ImU32 COLOR_BORDER = IM_COL32(0, 0, 0, 255 * 0.8);
    const ImU32 COLOR_BORDER2 = IM_COL32(255, 255, 255, 255 * 0.8);

public:
    explicit DecorationDragger(OutputVideoEditor *editor) : editor(editor) {};

    void startEditing(const std::shared_ptr<model::Decoration> &decoration);
    void update();

    void drawPlaceBox(ImRect rect, bool isPlaceHolder = false);

private:
    OutputVideoEditor *editor;
    bool isMoving;
    ImVec2 originalMouseX;
    ImVec2 originalPos;

    std::shared_ptr<model::Decoration> decoration;
};

}
}

#endif //DECORATIONDRAGGER_H
