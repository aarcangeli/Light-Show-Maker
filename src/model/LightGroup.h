#ifndef LIGHTGROUP_H
#define LIGHTGROUP_H

#include <memory>
#include <vector>
#include <string>
#include "Decoration.h"
#include "core.h"
#include "KeyPoint.h"

namespace sm {
namespace project {
class Canvas;

class LightGroup {
public:
    explicit LightGroup(Canvas *canvas);
    ~LightGroup();

    std::string name;
    std::vector<Decoration> decorations;
    std::vector<std::shared_ptr<KeyPoint>> keys;

    std::shared_ptr<KeyPoint> addKey(time_unit start, time_unit duration);

    // returns first index that starts at 'time' or after, if missing returns keys.size()
    size_t findIndex(time_unit time);

private:
    Canvas *canvas;
    bool sanityCheck();

    size_t findIndexIt(time_unit time, size_t min, size_t max);
};

}
}

#endif //LIGHTGROUP_H
