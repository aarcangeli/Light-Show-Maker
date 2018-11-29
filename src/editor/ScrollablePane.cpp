#define IMGUI_DEFINE_MATH_OPERATORS

#include "ScrollablePane.h"
#include "GLFW/glfw3.h"

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
    bool isContentHovered = IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
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
            scrollX -= io.MouseWheel * scrollAmount;
        } else {
            scrollY -= io.MouseWheel * scrollAmount;
        }
    }
    if (isDragging) {
        ImVec2 delta = io.MouseDelta;
        if (delta.x != 0 || delta.y != 0) {
            scrollX -= delta.x;
            scrollY -= delta.y;
        }
    }
    if (isContentHovered && IsKeyDown(GLFW_KEY_SPACE) && IsMouseDragging(0, 0)) {
        isDragging = true;
    }
    if (isDragging && !IsMouseDragging(0, 0)) {
        isDragging = false;
    }

    float center = io.MousePos.x - bounds.Min.x;
    float oldZoom = zoom.x;
    zoom = ImLerp(zoom, zoomTarget, 0.33f);
    scrollX = (scrollX + center) * (zoom.x / oldZoom) - center;

    float maxValueX = content.x - (bounds.Max.x - bounds.Min.x);
    if (scrollX > maxValueX) scrollX = maxValueX;
    if (scrollX < 0) scrollX = 0;
    float maxValueY = content.y - (bounds.Max.y - bounds.Min.y);
    if (scrollY > maxValueY) scrollY = maxValueY;
    if (scrollY < 0) scrollY = 0;

    SetScrollX(scrollX);
    SetScrollY(scrollY);
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
