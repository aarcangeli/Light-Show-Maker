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

class SelectionManager;

template <typename T>
class Selection {
    typedef typename std::vector<std::shared_ptr<T>> array_type;
    typedef typename array_type::const_iterator const_iterator;

    Selection(SelectionManager *manager, SELECTION_TYPE type) : manager(manager), type(type) {};
    const SELECTION_TYPE type;
    SelectionManager *manager;
    array_type currentSelection;
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
        setLastSelection();
    }

    void toggle(std::shared_ptr<T> item) {
        assert(item);
        if (!remove(item)) {
            currentSelection.push_back(item);
            item->isSelected = true;
        }
        setLastSelection();
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

    array_type getVector() {
        return currentSelection;
    }

    inline const_iterator begin() const noexcept { return currentSelection.cbegin(); }
    inline const_iterator end() const noexcept { return currentSelection.cend(); }

    void setLastSelection();
};

class SelectionManager {
public:
    SelectionManager()
            : layers(this, LAYER),
              decorations(this, DECORATION),
              keypoints(this, KEYPOINT) {};

    SELECTION_TYPE lastSelection;
    Selection<project::Layer> layers;
    Selection<project::Decoration> decorations;
    Selection<project::KeyPoint> keypoints;
};

template<typename T>
void Selection<T>::setLastSelection() {
    manager->lastSelection = type;
}

}


#endif //CHRISTMASLIGHTSHOWMAKER_SELECTION_H
