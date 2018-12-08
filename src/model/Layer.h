#ifndef LAYER_H
#define LAYER_H

#include <memory>
#include <vector>
#include <string>
#include "Decoration.h"
#include "core.h"
#include "KeyPoint.h"
#include "Serialization.h"

namespace sm {
namespace project {

class Layer {
public:
    Layer();
    ~Layer();

    std::string name;
    std::vector<std::shared_ptr<Decoration>> decorations;
    std::vector<std::shared_ptr<KeyPoint>> keys;
    // volatile
    bool isSelected = false;

    std::shared_ptr<KeyPoint> addKey(time_unit start, time_unit duration);
    void addKey(const std::shared_ptr<KeyPoint> &key);
    void removeKey(const std::shared_ptr<KeyPoint> &key);
    bool hasKey(const std::shared_ptr<KeyPoint> &key);

    // returns first index that starts at 'time' or after, if missing returns keys.size()
    size_t findIndex(time_unit time);

    float computeEasing(time_unit time);

    int32_t findIndex(const std::shared_ptr<KeyPoint> &key);

    void sortKeys();

    SERIALIZATION_START {
        ser.serialize("name", name);
        ser.serialize("decorations", decorations);
        ser.serialize("keys", keys);
    }

    bool sanityCheck();

private:
    size_t findIndexIt(time_unit time, size_t min, size_t max);
};

}
}

#endif //LAYER_H
