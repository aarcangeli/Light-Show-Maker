#ifndef CANVAS_H
#define CANVAS_H

#include "vector"
#include "memory"
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

    time_unit duration = 210 * TIME_UNITS;
    std::list<Decoration> decorations;
    std::vector<std::shared_ptr<LightGroup>> groups;

    std::shared_ptr<LightGroup> makeGroup();

    void deleteGroup(const std::shared_ptr<LightGroup> &group);

private:
    Project *project;
};

}
}

#endif //CANVAS_H
