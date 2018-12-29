#ifndef GLOBALHOTKEY_H
#define GLOBALHOTKEY_H

#include "core.h"
#include "imgui.h"
#include "GLFW/glfw3.h"

namespace sm {

class GlobalHotKey {
public:
    void update();

private:
    bool isSpaceHandled = false;
};

}


#endif //GLOBALHOTKEY_H
