#ifndef LIGHTGROUP_H
#define LIGHTGROUP_H

#include <list>
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

    std::string name;
    std::list<Decoration> decorations;
    std::list<KeyPoint> pins;

private:
    Canvas *canvas;
};

}
}

#endif //LIGHTGROUP_H
