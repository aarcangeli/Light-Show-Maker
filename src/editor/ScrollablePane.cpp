#define IMGUI_DEFINE_MATH_OPERATORS

#include "ScrollablePane.h"

using namespace sm;
using namespace sm::editor;
using namespace ImGui;

ScrollablePane::ScrollablePane() = default;

void ScrollablePane::scrollPaneBegin(const ImRect &bounds, const ImVec2 &content) {
    ImGuiStyle &style = GetStyle();
    ImGuiIO &io = GetIO();
    ImGuiWindow *window = GetCurrentWindow();

    oldPos = GetCursorScreenPos();
    SetCursorScreenPos(bounds.Min);

    bool isHovered = IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
    BeginChild(window->GetID(this), bounds.Max - bounds.Min, false,
               ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    scrollX = GetScrollX();
    scrollY = GetScrollY();
    Dummy(content);

    if (isHovered && io.MouseWheel != 0) {
        float scrollAmount = 3 * window->CalcFontSize();
        if (io.KeyAlt) {
            double factor = pow(1.1, io.MouseWheel);
            zoomTarget.x *= factor;
            zoomTarget.x = ImClamp(zoomTarget.x, 0.076278f, 49.785397f);
        } else if (io.KeyShift) {
            SetScrollX(GetScrollX() - io.MouseWheel * scrollAmount);
        } else {
            SetScrollY(GetScrollY() - io.MouseWheel * scrollAmount);
        }
    }

    zoom = ImLerp(zoom, zoomTarget, 0.33f);
}

void ScrollablePane::scrollPaneEnd() {
    EndChild();
    SetCursorPos(oldPos);
}

ImVec2 ScrollablePane::getOffset() const {
    return {scrollX, scrollY};
}

const ImVec2 &ScrollablePane::getScale() const {
    return zoom;
}
