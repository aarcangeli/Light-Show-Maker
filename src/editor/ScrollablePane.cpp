#define IMGUI_DEFINE_MATH_OPERATORS

#include "ScrollablePane.h"
#include "GLFW/glfw3.h"
#include "Application.h"

using namespace sm;
using namespace sm::editor;
using namespace ImGui;

void ScrollablePane::scrollPaneBegin(bool isHovered, const ImRect &bounds, const ImVec2 &content) {
    ImGuiStyle &style = GetStyle();
    ImGuiIO &io = GetIO();
    ImGuiWindow *window = GetCurrentWindow();

    oldPos = GetCursorScreenPos();
    SetCursorScreenPos(bounds.Min);

    BeginChild(window->GetID(this), bounds.Max - bounds.Min, false,
               ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    bool isContentHovered = IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
    if (!changeScroll) {
        scrollX = GetScrollX();
        scrollY = GetScrollY();
        changeScroll = false;
    }
    Dummy(content);

    if (isHovered && io.MouseWheel != 0) {
        float scrollAmount = 3 * window->CalcFontSize();
        if (io.KeyAlt) {
            double factor = pow(1.1, io.MouseWheel);
            zoomTarget.x *= factor;
            zoomTarget.x = ImClamp(zoomTarget.x, 0.04f, 200.f);
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
    if (!isDragging && isContentHovered && IsKeyDown(GLFW_KEY_SPACE) && IsMouseDragging(0, 0)) {
        isDragging = true;
    }
    if (isDragging && !IsMouseDragging(0, 0)) {
        isDragging = false;
    }

    float oldZoom = zoom.x;
    zoom = ImLerp(zoom, zoomTarget, 0.33f);
    if (io.MousePos.x != -FLT_MAX) {
        float center = io.MousePos.x - bounds.Min.x;
        scrollX = (scrollX + center) * (zoom.x / oldZoom) - center;
    }

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

void ScrollablePane::setOffset(const ImVec2 &scroll) {
    scrollX = scroll.x;
    scrollY = scroll.y;
    changeScroll = true;
}

void ScrollablePane::setScale(const ImVec2 &scale) {
    zoom = zoomTarget = scale;
}
