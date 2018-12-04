#ifndef APPLICATION_H
#define APPLICATION_H

#include "memory"
#include "core.h"
#include "Project.h"
#include <GLFW/glfw3.h>
#include <functional>
#include <imgui.h>
#include "ProjectWindow.h"
#include "AudioDevice.h"
#include "Player.h"

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

    void setLayerSelected(std::shared_ptr<project::LightGroup> layer);

    std::shared_ptr<project::LightGroup> layerSelected() { return selectedGroup; }

    void setDecorationSelected(std::shared_ptr<project::Decoration> decoration);

    std::shared_ptr<project::Decoration> decorationSelected() { return selectedDecoration; }

    void beginCommand(const std::string &name, bool mergeable = false);
    void endCommand();
    void stopMerging();
    void asyncCommand(const std::string &name, bool mergeable, const std::function<void()> &fn);

    void setAppHome(std::string home);

    std::string getPath(const std::string &pathes);
    void saveLastDirectory(std::string path);

private:
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
    bool dirtyStyle = false;

    std::shared_ptr<project::Project> proj;
    editor::ProjectWindow projectWindow;
    media::AudioDevice device;
    Player player;

    ImFont *loadFont(const char *start, const char *end, float size, bool fontAwesome) const;

    void applyTheme();

    std::shared_ptr<project::LightGroup> selectedGroup;
    std::shared_ptr<project::Decoration> selectedDecoration;
    std::string home;
    std::string iniPath;
    std::string autosavePath;

    void save(std::string filename);

    void load(std::string filename);

public:
    void error(const std::string &errorMsg) { projectWindow.showError(errorMsg); }

    Player &getPlayer() { return player; };
};

}

#endif //APPLICATION_H
