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

typedef int arduino_number;

class Layer {
public:
    Layer();
    ~Layer();

    std::string name;
    std::vector<std::shared_ptr<Decoration>> decorations;
    std::vector<std::shared_ptr<KeyPoint>> keys;
    arduino_number number = 0;
    std::string identifier;
    // volatile
    bool isSelected = false;

    std::shared_ptr<KeyPoint> addKey(time_unit start, time_unit duration);
    void addKey(const std::shared_ptr<KeyPoint> &key);
    void removeKey(const std::shared_ptr<KeyPoint> &key);
    bool hasKey(const std::shared_ptr<KeyPoint> &key);

    // returns first index that starts at 'time' or after, if missing returns keys.size()
    size_t findIndex(time_unit time);

    // returns first index that starts at 'time' or after, if missing returns keys.size()
    size_t findBefore(time_unit time);

    float computeEasing(time_unit time);

    int32_t findIndex(const std::shared_ptr<KeyPoint> &key);

    void sortKeys();

    SERIALIZATION_START {
        ser.serialize("name", name);
        ser.serialize("decorations", decorations);
        ser.serialize("keys", keys);
        ser.serialize("number", number);
        ser.serialize("identifier", identifier);
    }

    bool sanityCheck();

    void changeKeyStart(std::shared_ptr<KeyPoint> &key, time_unit newStart);

private:
    size_t findIndexIt(time_unit time, size_t min, size_t max);
};

}
}

#endif //LAYER_H
