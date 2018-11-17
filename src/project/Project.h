#ifndef PROJECT_H
#define PROJECT_H

#include "vector"
#include "Canvas.h"
#include "Settings.h"

namespace sm {
namespace project {

/**
 * Main class, contains the entire project structure
 */
class Project {
public:
    Project();

private:
    Canvas canvas;
    Settings settings;

};

}
}

#endif //PROJECT_H
