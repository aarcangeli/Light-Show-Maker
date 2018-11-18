#ifndef APPLICATION_H
#define APPLICATION_H

#include "memory"
#include "core.h"
#include "Editor.h"
#include "Project.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <imgui.h>

namespace sm {

class Application {
public:
    Application();

    // document managment
    void open(std::shared_ptr<project::Project> proj);
    void close();

    bool init();
    int runLoop();
    void cleanUp();

    bool showMaximized = false;
    int windowWidth = 1280;
    int windowHeight = 720;
    int theme = 1;

private:
    GLFWwindow *mainWindow = nullptr;
    ImGuiContext *ctx = nullptr;
    editor::Editor myEditor;
    std::shared_ptr<project::Project> proj;
    bool exit = false;

    void windowMenu();
    void applyTheme() const;
};

}

#endif //APPLICATION_H
