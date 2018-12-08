#include "KeypointDragger.h"
#include <imgui.h>
#include <Application.h>
#include <TimelineEditor.h>

using namespace std;
using namespace ImGui;
using namespace sm;
using namespace sm::project;
using namespace sm::editor;

void KeypointDragger::startDragging(const std::shared_ptr<KeyPoint> &key_,
                                    const std::shared_ptr<LightGroup> &owner_,
                                    DragType type_,
                                    float timeScale_) {
    assert(type_ > NOTHING && type_ < DragType::SIZE);
    key = key_;
    owner = owner_;
    type = type_;
    timeScale = timeScale_;
    originalStart = key->start;
    originalDuration = key->duration;
    originalMouseX = GetIO().MousePos.x;
    dragging = true;
    updateBounds();
}

void KeypointDragger::updateBounds() {
    int32_t idx = owner->findIndex(key);
    assert(idx >= 0);
    if (idx > 0) {
        shared_ptr<KeyPoint> &prev = owner->keys[idx - 1];
        minBound = prev->start + prev->duration;
    } else {
        minBound = 0;
    }
    if (idx + 1 < owner->keys.size()) {
        maxBound = owner->keys[idx + 1]->start;
    } else {
        maxBound = max_time;
    }
}

void KeypointDragger::update() {
    if (!dragging) return;

    ImGuiIO &io = GetIO();
    if (io.MouseDelta.x != 0 || io.MouseDelta.y != 0) {
        float delta = io.MousePos.x - originalMouseX;
        time_unit diff = static_cast<time_unit>(delta / timeScale);

        time_unit start = originalStart;
        time_unit end = originalStart + originalDuration;

        gApp->beginCommand("Move/Resize key point", true);

        time_unit reposedStart;
        time_unit reposedDuration = end - start;
        int32_t layerIdx;
        if (type == MOVE && editor->findPlacableKeyPos(io.MousePos, reposedStart, reposedDuration, layerIdx)) {
            auto &newOwner = editor->getCanvas()->groups[layerIdx];
            if (newOwner != owner || reposedStart < minBound || reposedStart + reposedDuration > maxBound) {
                owner->removeKey(key);
                owner = newOwner;
                key->start = reposedStart;
                key->duration = reposedDuration;
                owner->addKey(key);
                updateBounds();
            }
        }

        if (type == MOVE && io.KeyShift) {
            diff = 0;
        }

        switch (type) {
            case MOVE:
                if (start + diff < minBound) {
                    diff = minBound - start;
                }
                if (end + diff > maxBound) {
                    diff = maxBound - end;
                }
                start += diff;
                end += diff;
                if (io.KeyAlt) {
                    time_unit duration = end - start;
                    start = editor->moveSnapped(start, [=](time_unit dest, shared_ptr<project::KeyPoint> it) -> bool {
                        return dest >= minBound && dest <= end && it != key;
                    });
                    end = start + duration;
                }
                break;
            case RESIZE_BEGIN:
                start += diff;
                if (io.KeyAlt) {
                    start = editor->moveSnapped(start, [=](time_unit dest, shared_ptr<project::KeyPoint> it) -> bool {
                        return dest >= minBound && dest <= end && it != key;
                    });
                }
                if (start > end) start = end;
                if (start < minBound) start = minBound;
                break;
            case RESIZE_END:
                end += diff;
                if (io.KeyAlt) {
                    end = editor->moveSnapped(end, [=](time_unit dest, shared_ptr<project::KeyPoint> it) -> bool {
                        return dest >= start && dest <= maxBound && it != key;
                    });
                }
                if (end < start) end = start;
                if (end > maxBound) end = maxBound;
                break;
            default:
                assert(false);
        }

        key->start = start;
        key->duration = end - start;

        assert(owner->sanityCheck()); // the order should not have been changed

        gApp->endCommand();
    }

    if (!IsMouseDown(0)) {
        dragging = false;
        key.reset();
        owner.reset();
        gApp->stopMerging();
        type = NOTHING;
    }
}

size_t KeypointDragger::getPrev(size_t idx) const {
    size_t prev = idx;
    if (prev > 0) prev--;
    if (prev > 0 && owner->keys[prev] == key) prev--;
    return prev;
}

size_t KeypointDragger::getNext(size_t idx) {
    size_t next = idx;
    if (next < owner->keys.size() && owner->keys[next] == key) next++;
    return next;
}
