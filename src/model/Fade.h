#ifndef FADE_H
#define FADE_H

#include "core.h"

namespace sm {
namespace project {

enum FADE_TYPE {
    LINEAR,
    EXPONENTIAL,
};

class Fade {
public:
    FADE_TYPE type = EXPONENTIAL;
    time_unit duration = static_cast<time_unit>(TIME_UNITS * 0.2);
};

}
}

#endif //FADE_H
