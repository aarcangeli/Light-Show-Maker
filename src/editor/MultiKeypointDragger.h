#ifndef MULTIKEYPOINTDRAGGER_H
#define MULTIKEYPOINTDRAGGER_H

#include <Canvas.h>
#include "imgui.h"
#include "Layer.h"
#include "KeyPoint.h"
#include "KeypointDragger.h"

namespace sm {
namespace editor {

class TimelineEditor;

class MultiKeypointDragger {
    const int DRAG_MIN = 5;

public:
    explicit MultiKeypointDragger(TimelineEditor *editor) : editor(editor) {};

private:
    TimelineEditor *editor;
    DragType type;

    bool dragging = false;
    float timeScale;
    time_unit lastDiff;
    std::vector<std::shared_ptr<project::KeyPoint>> keys;
    project::Canvas *owner;
    ImVec2 originalMouse;
    bool dragConfirmed = false;

public:
    void update();

    void startDragging(const std::vector<std::shared_ptr<project::KeyPoint>> &key,
                       project::Canvas *owner,
                       DragType type,
                       float timeScale);

    bool isDragging() { return dragging; }
    void snapTime(time_unit source, time_unit &deltaTime) const;
};

}
}

#include <AudioDevice.h>

#endif //MULTIKEYPOINTDRAGGER_H
