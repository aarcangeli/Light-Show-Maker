#ifndef LIGHTGROUP_H
#define LIGHTGROUP_H

#include <list>
#include "Decoration.h"
#include "core.h"
#include "KeyPoint.h"

namespace sm {
namespace project {
class Canvas;

class LightGroup {
public:
    explicit LightGroup(Canvas *canvas);

private:
    Canvas *canvas;
    std::list<Decoration> decorations;
    std::list<KeyPoint> pins;
};

}
}

#endif //LIGHTGROUP_H
