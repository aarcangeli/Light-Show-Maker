#include "Application.h"
#include "dpi.h"
#include <cstdio>

using namespace sm;

static void glfw_error_callback(int error, const char *description);

int main(int argc, char **argv) {
    float source = 1e20f; // a number logically large for a 16 bit signed integer
    int32_t converted = (int32_t) source;
    printf("converted: %i", converted);
    return 0;

    glfwSetErrorCallback(glfw_error_callback);
    setupDpi();

    Application &app = *new Application;

    // todo: parse command line

    if (!app.init()) {
        printf("Cannot initialize application\n");
        return 1;
    }

    if (argc == 2) {
        app.load(argv[1]);
    }

    int res = app.runLoop();
    app.cleanUp();
    return res;
}

static void glfw_error_callback(int error, const char *description) {
    printf("Glfw Error %d: %s\n", error, description);
}
