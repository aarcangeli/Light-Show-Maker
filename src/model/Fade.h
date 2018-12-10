#ifndef FADE_H
#define FADE_H

#include "core.h"
#include "Serialization.h"

namespace sm {
namespace project {

enum FADE_TYPE {
    LINEAR,
    EXPONENTIAL,
    SIN,
    SIN_DOUBLE,
};

class Fade {
public:
    FADE_TYPE type = SIN;
    time_unit duration = static_cast<time_unit>(TIME_UNITS * 0.2);
    float exponent = 2;

    SERIALIZATION_START {
        ser.serializeEnum("type", type);
        ser.serialize("duration", duration);
        ser.serialize("exponent", exponent);
    }
};

}
}

#endif //FADE_H
