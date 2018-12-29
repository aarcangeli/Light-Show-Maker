#ifndef PROJECT_H
#define PROJECT_H

#include "vector"
#include "string"
#include "Canvas.h"
#include "Settings.h"
#include "Serialization.h"

namespace sm {
namespace model {

/**
 * Main class, contains the entire project structure
 */
class Project {
public:
    Project();

    std::string name;
    Canvas canvas;
    Settings settings;

    SERIALIZATION_START {
        ser.serialize("name", name);
        ser.serialize("canvas", canvas);
        ser.serialize("settings", settings);
    }
};

}
}

#endif //PROJECT_H
