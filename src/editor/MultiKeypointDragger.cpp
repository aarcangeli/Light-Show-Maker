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

        gApp->beginCommand("Move/Resize multiple key point", true);

        if (io.KeyShift && type == DRAG_MOVE) {
            auto first = keys[keys.size() - 1];
            time_unit from = deltaTime + first->start;
            time_unit dest1;
            dest1 = editor->moveSnapped(from, [&](time_unit dest, shared_ptr<project::KeyPoint> it) -> bool {
                for (auto &k : keys) {
                    if (it == k) {
                        return false;
                    }
                }
                return true;
            });
            deltaTime += dest1 - from;
        }

        if (type == DRAG_MOVE) {
            for (auto &l : keys) {
                l->start += deltaTime;
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
