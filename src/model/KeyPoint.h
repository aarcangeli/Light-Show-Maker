#ifndef KEYPOINT_H
#define KEYPOINT_H

#include "core.h"
#include "Fade.h"

namespace sm {
namespace project {
class LightGroup;

class KeyPoint {
public:
    explicit KeyPoint(LightGroup *group);

    time_unit start;
    time_unit length;
    Fade fadeStart, fadeEnd;

private:
    LightGroup *group;
};


}
}

#endif //KEYPOINT_H
