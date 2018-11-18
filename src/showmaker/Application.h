#ifndef APPLICATION_H
#define APPLICATION_H

#include "memory"
#include "core.h"
#include "Project.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <imgui.h>
#include "ProjectWindow.h"

namespace sm {

class Application {
public:
    Application();

    void open(std::shared_ptr<project::Project> proj);
    void close();

    bool init();
    int runLoop();
    void cleanUp();

    bool showMaximized = false;
    int windowWidth = 1280;
    int windowHeight = 720;
    int theme = 1;

    void applyTheme() const;

private:
    GLFWwindow *mainWindow = nullptr;
    ImGuiContext *ctx = nullptr;
    bool exit = false;
    bool closeProject = false;

    std::shared_ptr<project::Project> proj;
    ProjectWindow projectWindow;

};

}

#endif //APPLICATION_H
