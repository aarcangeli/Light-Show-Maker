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

    Project *getProject() { return project; }

    time_unit duration = 0;
    std::list<Decoration> decorations;
    std::list<LightGroup> groups;

private:
    Project *project;
};

}
}

#endif //CANVAS_H
