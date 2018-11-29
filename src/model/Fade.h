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
    FADE_TYPE type;
    time_unit duration;
};

}
}

#endif //FADE_H
