#ifndef TIMELINEEDITOR_H
#define TIMELINEEDITOR_H

#define IMGUI_DEFINE_MATH_OPERATORS

#include "Canvas.h"
#include <imgui.h>

namespace sm {
namespace editor {

class TimelineEditor {
    const int scrollBarHeight = 14;
    // multiplier of zoom and fontsize
    const float TIME_WIDTH = 1;

public:
    TimelineEditor();
    void reset();
    void editorOf(project::Canvas canvas);

private:
    float zoom;
    float zoomTarget;

    ImU32 setAlpha(ImU32 color, double alpha);
};

}
}

#endif //TIMELINEEDITOR_H
