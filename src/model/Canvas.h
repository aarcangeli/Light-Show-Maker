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
    std::string audioFile;

    std::shared_ptr<Layer> makeGroup();

    void deleteGroup(const std::shared_ptr<Layer> &group);
    int findGroupIndexWith(const std::shared_ptr<KeyPoint> &point);
    std::shared_ptr<Layer> findGroupWith(const std::shared_ptr<KeyPoint> &point);

    SERIALIZATION_START {
        ser.serialize("decorations", decorations);
        ser.serialize("groups", groups);
        ser.serialize("audioFile", audioFile);
    }

private:
    Project *project;
};

}
}

#endif //CANVAS_H
