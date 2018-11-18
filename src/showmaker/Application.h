#ifndef APPLICATION_H
#define APPLICATION_H

#include "core.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

namespace sm {

class Application {
public:
    bool init();
    int runLoop();
    void cleanUp();

    bool showMaximized = false;
    int windowWidth = 1280;
    int windowHeight = 720;

private:
    GLFWwindow *mainWindow = nullptr;
    ImGuiContext *ctx = nullptr;
};

}

#endif //APPLICATION_H
