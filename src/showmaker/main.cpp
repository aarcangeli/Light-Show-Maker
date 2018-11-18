#include "Application.h"
#include <cstdio>



#ifdef WIN32
#include "Windows.h"
#endif

using namespace sm;

static void glfw_error_callback(int error, const char *description);

#ifdef WIN32
int WinMain(HINSTANCE hInst, HINSTANCE hPreInst, LPSTR nCmdLine, int nCmdShow)
#else
int main(int argc, char **argv)
#endif
{
    glfwSetErrorCallback(glfw_error_callback);

    Application app{};

    // todo: parse command line

    if (!app.init()) {
        printf("Cannot initialize application\n");
        return 1;
    }

    int res = app.runLoop();
    app.cleanUp();
    return res;
}

static void glfw_error_callback(int error, const char *description) {
    printf("Glfw Error %d: %s\n", error, description);
}
