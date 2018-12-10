#include "Application.h"
#include "dpi.h"
#include <cstdio>

using namespace sm;

static void glfw_error_callback(int error, const char *description);

int main(int argc, char **argv) {
    glfwSetErrorCallback(glfw_error_callback);
    setupDpi();

    Application &app = *new Application;

    app.setAppHome(argv[0]);

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
