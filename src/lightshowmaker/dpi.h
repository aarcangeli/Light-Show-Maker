#ifndef DPI_H
#define DPI_H

#include <GLFW/glfw3.h>

namespace sm {

void setupDpi();

float getDpiForWindow(GLFWwindow *window);

}

#endif //DPI_H
