#ifndef KEYPOINTDRAGGER_H
#define KEYPOINTDRAGGER_H

#include "imgui.h"
#include "Layer.h"
#include "KeyPoint.h"

namespace sm {
namespace editor {

class TimelineEditor;

class KeypointDragger {
    const int DRAG_MIN = 5;

public:
    explicit KeypointDragger(TimelineEditor *editor) : editor(editor) {};

    enum DragType {
        NOTHING,
        MOVE,
        RESIZE_BEGIN,
        RESIZE_END,
        SIZE,
    };

private:
    TimelineEditor *editor;
    DragType type;

    bool dragging = false;
    float timeScale;
    std::shared_ptr<project::KeyPoint> key;
    std::shared_ptr<project::Layer> owner;
    sm::time_unit originalStart;
    sm::time_unit originalDuration;
    ImVec2 originalMouse;
    bool dragConfirmed = false;

    sm::time_unit minBound, maxBound;

public:
    void update();

    void startDragging(const std::shared_ptr<project::KeyPoint> &key,
                       const std::shared_ptr<project::Layer> &owner,
                       DragType type,
                       float timeScale);

    bool isDragging() { return dragging; }

    std::shared_ptr<project::KeyPoint> getElement() { return key; }
    DragType getType() { return type; }
    size_t getPrev(size_t idx) const;
    size_t getNext(size_t idx);

    void updateBounds();
};

}
}

#include <AudioDevice.h>

#endif //KEYPOINTDRAGGER_H
