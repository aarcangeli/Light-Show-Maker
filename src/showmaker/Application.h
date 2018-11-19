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
    const float FONT_BASE = 15;
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

    int getTheme() const {
        return theme;
    }

    void setTheme(int theme) {
        if (theme >= 0 && theme < 3) {
            Application::theme = theme;
            dirtyStyle = true;
        }
    }

private:
    int theme = 1;

    GLFWwindow *mainWindow = nullptr;
    ImGuiContext *ctx = nullptr;
    bool exit = false;
    bool closeProject = false;
    float dpi = 1;
    bool dirtyStyle = false;

    std::shared_ptr<project::Project> proj;
    ProjectWindow projectWindow;

    ImFont *loadFont(const char *start, const char *end, float size) const;

    void applyTheme();
};

}

#endif //APPLICATION_H
