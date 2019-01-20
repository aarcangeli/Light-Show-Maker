#ifndef KEYPOINT_H
#define KEYPOINT_H

#include "core.h"
#include "Fade.h"
#include "Serialization.h"
#include "SelectableItem.h"

namespace sm {
namespace model {
class Layer;

class KeyPoint : public SelectableItem<KeyPoint> {
public:
    explicit KeyPoint();

    time_unit start;
    time_unit duration;
    Fade fadeStart, fadeEnd;
    float maxWeight = 1;

    float computeEasing(time_unit time);

    SERIALIZATION_START {
        SelectableItem::serializeBase(ser);
        ser.serialize("start", start);
        ser.serialize("duration", duration);
        ser.serialize("fadeStart", fadeStart);
        ser.serialize("fadeEnd", fadeEnd);
        ser.serialize("maxWeight", maxWeight);
    }

private:
    float computeFade(bool isRight, const Fade &fade, time_unit time) const;

};


}
}

#endif //KEYPOINT_H
