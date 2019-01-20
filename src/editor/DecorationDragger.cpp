#define IMGUI_DEFINE_MATH_OPERATORS

#include "DecorationDragger.h"
#include "OutputVideoEditor.h"
#include "GLFW/glfw3.h"
#include "Application.h"

using namespace sm;
using namespace sm::editor;
using namespace sm::model;
using namespace ImGui;

void DecorationDragger::startEditing(const std::shared_ptr<Decoration> &decoration_) {
    decoration = decoration_;
    if (!decoration) return;
    isMoving = IsMouseDown(0);
    originalMouseX = GetMousePos();
    originalPos = ImVec2(decoration->posX, decoration->posY);
}

void DecorationDragger::update() {
    if (!decoration) return;

    ImGuiIO &io = GetIO();

    ImRect region = editor->getDecorationRegion(decoration);
    drawPlaceBox(region);

    if (isMoving) {
        if (io.MouseDelta.x != 0 || io.MouseDelta.y != 0) {
            ImVec2 delta = originalPos + (GetMousePos() - originalMouseX) / editor->getLogicalScale();
            if (gApp->beginCommand("Move decoration", true)) {
                decoration->posX = delta.x;
                decoration->posY = delta.y;
                gApp->endCommand();
            }
        }
        if (!IsMouseDown(0)) {
            isMoving = false;
            gApp->stopMerging();
        }
    }

    if (editor->isFocused() && IsKeyPressed(GLFW_KEY_DELETE)) {
        editor->deleteDecoration(decoration);
        startEditing(nullptr);
    }
}

void DecorationDragger::drawPlaceBox(ImRect region, bool isPlaceHolder) {
    if (isPlaceHolder && isMoving) return;
    const ImVec2 &dpiVec = ImVec2(dpi, dpi);
    ImDrawList *drawList = GetWindowDrawList();
    drawList->AddRect(region.Min, region.Max, COLOR_BORDER, dpi);
    drawList->AddRect(region.Min - dpiVec, region.Max + dpiVec, COLOR_BORDER2, 0, 0, dpi);
}
