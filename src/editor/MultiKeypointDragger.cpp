#define IMGUI_DEFINE_MATH_OPERATORS
#include "MultiKeypointDragger.h"
#include <imgui.h>
#include <Application.h>
#include <TimelineEditor.h>
#include <Canvas.h>

using namespace std;
using namespace ImGui;
using namespace sm;
using namespace sm::project;
using namespace sm::editor;

void MultiKeypointDragger::startDragging(const std::vector<std::shared_ptr<project::KeyPoint>> &keys_,
                                         project::Canvas *owner_,
                                         DragType type_,
                                         float timeScale_) {
    assert(keys_.size() > 1);
    assert(type_ > DRAG_NOTHING && type_ < DRAG_SIZE);
    keys = keys_;
    owner = owner_;
    type = type_;
    timeScale = timeScale_;
    originalMouse = GetIO().MousePos;
    dragging = true;
    dragConfirmed = false;
    lastDiff = 0;
}

void MultiKeypointDragger::update() {
    if (!dragging) return;

    if (!IsMouseDown(0)) {
        dragging = false;
        keys.resize(0);
        owner = nullptr;
        gApp->stopMerging();
        type = DRAG_NOTHING;
        return;
    }

    ImGuiIO &io = GetIO();
    if (io.MouseDelta.x != 0 || io.MouseDelta.y != 0) {
        ImVec2 delta = io.MousePos - originalMouse;
        if (std::abs(delta.x) > dpi * DRAG_MIN || std::abs(delta.y) > dpi * DRAG_MIN) {
            dragConfirmed = true;
        }
        if (!dragConfirmed) return;
        time_unit mouseMoved = static_cast<time_unit>(delta.x / timeScale);
        time_unit deltaTime = mouseMoved - lastDiff;
        auto last = keys[keys.size() - 1];

        gApp->beginCommand("Move/Resize multiple key point", true);

        if (io.KeyShift && type == DRAG_MOVE) {
            snapTime(last->start, deltaTime);
        }

        if (type == DRAG_MOVE) {
            for (auto &l : keys) {
                l->start += deltaTime;
            }
        }
        if (type == DRAG_RESIZE_BEGIN) {
            if (io.KeyShift) snapTime(last->start, deltaTime);
            for (auto &l : keys) {
                if (l->duration - deltaTime < 0) {
                    deltaTime = l->duration;
                }
            }
            for (auto &l : keys) {
                l->start += deltaTime;
                l->duration -= deltaTime;
            }
        }
        if (type == DRAG_RESIZE_END) {
            if (io.KeyShift) snapTime(last->start + last->duration, deltaTime);
            for (auto &l : keys) {
                if (l->duration + deltaTime < 0) {
                    deltaTime = -l->duration;
                }
            }
            for (auto &l : keys) {
                l->duration += deltaTime;
            }
        }
        lastDiff += deltaTime;

        // sort all
        for (auto &l : owner->groups) {
            l->sortKeys();
            l->sanityCheck();
        }

        gApp->endCommand();
    }
}

void MultiKeypointDragger::snapTime(time_unit source, time_unit &deltaTime) const {
    time_unit from = deltaTime + source;
    time_unit dest1;
    dest1 = editor->moveSnapped(from, [&](time_unit dest, shared_ptr<KeyPoint> it) -> bool {
        for (auto &k : keys) {
            if (it == k) {
                return false;
            }
        }
        return true;
    });
    deltaTime += dest1 - from;
}
