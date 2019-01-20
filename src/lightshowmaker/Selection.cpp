#include <utility>

#include "Selection.h"
#include "Application.h"
#include "map"

using namespace sm;
using namespace sm::model;

void SelectionManager::init(std::shared_ptr<model::Project> _proj) {
    proj = std::move(_proj);
    reset();
    layers.init(proj);
    decorations.init(proj);
    keypoints.init(proj);
}


template<>
void Selection<Layer>::unrollSelection(const std::vector<int64_t> &selection) {
    currentSelection.resize(selection.size());
    if (!selection.empty()) {
        for (auto &layer : proj->canvas.groups) {
            syncSelection(selection, layer);
        }
    }
    sanitize();
}

template<>
void Selection<Decoration>::unrollSelection(const std::vector<int64_t> &selection) {
    currentSelection.resize(selection.size());
    if (!selection.empty()) {
        for (auto &dec : proj->canvas.decorations) {
            syncSelection(selection, dec);
        }
        for (auto &layer : proj->canvas.groups) {
            for (auto &dec : layer->decorations) {
                syncSelection(selection, dec);
            }
        }
    }
    sanitize();
}

template<>
void Selection<KeyPoint>::unrollSelection(const std::vector<int64_t> &selection) {
    currentSelection.resize(selection.size());
    if (!selection.empty()) {
        for (auto &layer : proj->canvas.groups) {
            for (auto &key : layer->keys) {
                syncSelection(selection, key);
            }
        }
    }
    sanitize();
}
