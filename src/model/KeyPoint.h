#ifndef KEYPOINT_H
#define KEYPOINT_H

#include "core.h"
#include "Fade.h"
#include "Serialization.h"

namespace sm {
namespace project {
class Layer;

class KeyPoint {
public:
    explicit KeyPoint();

    time_unit start;
    time_unit duration;
    Fade fadeStart, fadeEnd;
    // volatile
    bool isSelected = false;

    float computeEasing(time_unit time);

    SERIALIZATION_START {
        ser.serialize("start", start);
        ser.serialize("duration", duration);
        ser.serialize("fadeStart", fadeStart);
        ser.serialize("fadeEnd", fadeEnd);
    }

private:
    float computeFade(bool isRight, const Fade &fade, time_unit time) const;

};


}
}

#endif //KEYPOINT_H
