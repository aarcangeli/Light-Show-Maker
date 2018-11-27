#ifndef SCROLLABLEPANE_H
#define SCROLLABLEPANE_H

#include "imgui.h"
#include "imgui_internal.h"

namespace sm {
namespace editor {

class ScrollablePane {
public:
    ScrollablePane();

    void scrollPaneBegin(const ImRect &bounds, const ImVec2 &content);
    void scrollPaneEnd();

    ImVec2 getOffset() const;
    const ImVec2 & getScale() const;

private:
    ImVec2 oldPos;
    float scrollX, scrollY;

    ImVec2 zoom = {1, 1};
    ImVec2 zoomTarget = {1, 1};
};

}
}

#endif //SCROLLABLEPANE_H
