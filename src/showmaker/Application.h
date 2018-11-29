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
#include "AudioDevice.h"

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
    std::string lastDirectory = "";
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

    void layerSelected(std::shared_ptr<project::LightGroup> layer);

    void beginCommand(const std::string &name, bool mergeable = false);
    void endCommand();
    void stopMerging();
    void asyncCommand(const std::string &name, bool mergeable, const std::function<void()> &fn);

    void setAppHome(std::string home);

private:
    int theme = 1;

    struct AppCommand {
        std::string name;
        bool mergeable;
        std::function<void()> fn;
    };

    std::vector<AppCommand> commands;

    GLFWwindow *mainWindow = nullptr;
    ImGuiContext *ctx = nullptr;
    bool exit = false;
    bool closeProject = false;
    float dpi = 1;
    bool dirtyStyle = false;

    std::shared_ptr<project::Project> proj;
    ProjectWindow projectWindow;
    media::AudioDevice device;

    ImFont *loadFont(const char *start, const char *end, float size, bool fontAwesome) const;

    void applyTheme();

    std::shared_ptr<project::LightGroup> selected;
    std::string home;
    std::string iniPath;
    std::string autosavePath;

    void save(std::string filename);

    void load(std::string filename);
};

}

#endif //APPLICATION_H
