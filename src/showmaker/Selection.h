#ifndef CHRISTMASLIGHTSHOWMAKER_SELECTION_H
#define CHRISTMASLIGHTSHOWMAKER_SELECTION_H

#include "cassert"
#include "memory"
#include "vector"
#include "Layer.h"
#include "Decoration.h"
#include "KeyPoint.h"

namespace sm {

enum SELECTION_TYPE {
    NOTHING,
    LAYER,
    DECORATION,
    KEYPOINT,
};

template <typename T>
class Selection {
    Selection(SELECTION_TYPE type) : type(type) {};
    const SELECTION_TYPE type;
    std::vector<std::shared_ptr<T>> currentSelection;
    friend class SelectionManager;

public:
    Selection(const Selection &) = delete;  // non construction-copyable
    Selection &operator=(const Selection &) = delete;  // non copyable

    void set(std::shared_ptr<T> item) {
        assert(item);
        reset();
        currentSelection.resize(1);
        currentSelection[0] = item;
        item->isSelected = true;
    }

    void toggle(std::shared_ptr<T> item) {
        assert(item);
        if (!remove(item)) {
            currentSelection.push_back(item);
            item->isSelected = true;
        }
    }

    bool remove(std::shared_ptr<T> item) {
        assert(item);
        if (item->isSelected) {
            auto it = std::find(currentSelection.begin(), currentSelection.end(), item);
            if (it != currentSelection.end()) {
                currentSelection.erase(it);
                item->isSelected = false;
                return true;
            }
        }
        return false;
    }

    inline void reset() {
        for (auto &it : currentSelection) {
            it->isSelected = false;
        }
        currentSelection.resize(0);
    }

    inline size_t size() {
        return currentSelection.size();
    }

    inline std::shared_ptr<T> &operator[](size_t itm) {
        return currentSelection[itm];
    }

    inline const std::shared_ptr<T> &operator[](size_t itm) const {
        return currentSelection[itm];
    }

    inline bool empty() const {
        return currentSelection.empty();
    }

    std::vector<std::shared_ptr<T>> getVector() {
        return currentSelection;
    }
};

class SelectionManager {
public:
    SelectionManager()
            : layers(LAYER),
              decorations(DECORATION),
              keypoints(KEYPOINT) {};

    Selection<project::Layer> layers;
    Selection<project::Decoration> decorations;
    Selection<project::KeyPoint> keypoints;
};

}


#endif //CHRISTMASLIGHTSHOWMAKER_SELECTION_H
