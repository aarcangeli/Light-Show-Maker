#ifndef SELECTION_H
#define SELECTION_H

#include "cassert"
#include "memory"
#include "vector"
#include "Layer.h"
#include "Decoration.h"
#include "KeyPoint.h"
#include "Project.h"

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

    int64_t nextId = 0;
    std::shared_ptr<model::Project> proj;

    void unrollSelection(const std::vector<int64_t> &selection);

    // remove null objects
    void sanitize() {
        auto it = currentSelection.begin();
        while (it != currentSelection.end()) {
            if (*it == nullptr) {
                it = currentSelection.erase(it);
            } else {
                it++;
            }
        }
    }

    void syncSelection(const std::vector<int64_t> &selection, const std::shared_ptr<T> &item) {
        int64_t id = item->id;
        if (id >= 0) {
            size_t i = 0;
            for (int64_t it : selection) {
                if (it == id) {
                    item->isSelected = true;
                    currentSelection[i] = item;
                }
                i++;
            }
        }
    }

public:
    Selection(const Selection &) = delete;  // non construction-copyable
    Selection &operator=(const Selection &) = delete;  // non copyable

    void init(std::shared_ptr<model::Project> _proj) {
        proj = std::move(_proj);
    }

    void set(std::shared_ptr<T> item) {
        assert(item);
        stopMerging();
        reset();
        currentSelection.resize(1);
        currentSelection[0] = item;
        item->isSelected = true;
        setLastSelection();
    }

    void toggle(std::shared_ptr<T> item) {
        assert(item);
        stopMerging();
        if (!remove(item)) {
            currentSelection.push_back(item);
            item->isSelected = true;
        }
        setLastSelection();
    }

    void add(std::shared_ptr<T> item) {
        assert(item);
        stopMerging();
        if (!item->isSelected) {
            currentSelection.push_back(item);
            item->isSelected = true;
        }
        setLastSelection();
    }

    bool remove(std::shared_ptr<T> item) {
        assert(item);
        stopMerging();
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

    /**
     * Remove all items from selection
     */
    inline void reset() {
        stopMerging();
        for (auto &it : currentSelection) {
            it->isSelected = false;
        }
        currentSelection.resize(0);
    }

    inline size_t size() const {
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

    array_type getVector() const {
        // copy
        return currentSelection;
    }

    inline const_iterator begin() const noexcept { return currentSelection.cbegin(); }
    inline const_iterator end() const noexcept { return currentSelection.cend(); }

    void setLastSelection();
    void stopMerging();

    SERIALIZATION_START {
        std::vector<int64_t> selection;
        if (ser.SERIALIZING) {
            for (auto &it : currentSelection) {
                int64_t &id = it->id;
                if (id < 0) {
                    id = nextId++;
                }
                selection.push_back(id);
            }
        }
        ser.serialize("selection", selection);
        if (ser.DESERIALIZING) {
            reset();
            unrollSelection(selection);
        }
    };
    std::map<int64_t, size_t> buildMap(const std::vector<int64_t> &selection) const;
};

class SelectionManager {
public:
    SelectionManager()
            : layers(this, LAYER),
              decorations(this, DECORATION),
              keypoints(this, KEYPOINT) {};

    SELECTION_TYPE lastSelection = NOTHING;
    Selection<model::Layer> layers;
    Selection<model::Decoration> decorations;
    Selection<model::KeyPoint> keypoints;

    void reset() {
        layers.reset();
        decorations.reset();
        keypoints.reset();
    }

    void init(std::shared_ptr<model::Project> proj);

    SERIALIZATION_START {
        ser.serializeEnum("lastSelection", lastSelection);
        ser.serialize("layers", layers);
        ser.serialize("decorations", decorations);
        ser.serialize("keypoints", keypoints);
    };

    std::shared_ptr<model::Project> proj;

    void stopMerging();
};

template<typename T>
void Selection<T>::setLastSelection() {
    manager->lastSelection = type;
}

template<typename T>
void Selection<T>::stopMerging() {
    manager->stopMerging();
}

}


#endif //SELECTION_H
