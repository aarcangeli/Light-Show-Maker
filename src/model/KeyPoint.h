#ifndef KEYPOINT_H
#define KEYPOINT_H

#include "core.h"
#include "Fade.h"
#include "Serialization.h"

namespace sm {
namespace project {
class LightGroup;

class KeyPoint {
public:
    explicit KeyPoint();

    time_unit start;
    time_unit duration;
    Fade fadeStart, fadeEnd;

    SERIALIZATION_START {
        ser.serialize("start", start);
        ser.serialize("duration", duration);
        ser.serialize("fadeStart", fadeStart);
        ser.serialize("fadeEnd", fadeEnd);
    }

};


}
}

#endif //KEYPOINT_H
