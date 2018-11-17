#include "Application.h"

#include <cstdio>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace sm;

static void glfw_error_callback(int error, const char *description) {
    printf("Glfw Error %d: %s\n", error, description);
}

int Application::runLoop() {
    GLFWwindow *window;

    glfwSetErrorCallback(glfw_error_callback);

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
