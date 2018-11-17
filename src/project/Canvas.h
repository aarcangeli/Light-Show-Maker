#ifndef CANVAS_H
#define CANVAS_H

#include "core.h"
#include "list"
#include "LightGroup.h"
#include "Decoration.h"

namespace sm {
namespace project {
class Project;

/**
 * A canvas contains
 */
class Canvas {
public:
    explicit Canvas(Project *parent);

private:
    Project *project;
    time_unit duration;
    std::list<Decoration> decorations;
    std::list<LightGroup> groups;
};

}
}

#endif //CANVAS_H
