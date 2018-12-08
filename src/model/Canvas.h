#ifndef CANVAS_H
#define CANVAS_H

#include "vector"
#include "memory"
#include "core.h"
#include "list"
#include "Layer.h"
#include "Decoration.h"
#include "Serialization.h"

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
    std::vector<std::shared_ptr<Decoration>> decorations;
    std::vector<std::shared_ptr<Layer>> groups;

    std::shared_ptr<Layer> makeGroup();

    void deleteGroup(const std::shared_ptr<Layer> &group);

    SERIALIZATION_START {
        ser.serialize("decorations", decorations);
        ser.serialize("groups", groups);
    }

private:
    Project *project;
};

}
}

#endif //CANVAS_H
