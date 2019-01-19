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
#include "GlobalHotKey.h"
#include "Selection.h"
#include "ResourceManager.h"
#include <path.hpp>

namespace sm {

class Application {
    const float FONT_BASE = 15;
public:
    Application();

    void open(std::shared_ptr<model::Project> proj);
    void close();

    bool init();
    int runLoop();
    void cleanUp();

    bool showMaximized = false;
    std::string lastDirectory = "";
    int windowWidth = 1280;
    int windowHeight = 720;
    std::string filename;

    void beginCommand(const std::string &name, bool mergeable = false);
    void endCommand();
    void stopMerging();
    void asyncCommand(const std::string &name, bool mergeable, const std::function<void()> &fn);

    std::string getPath(const std::string &pathes, bool isSave);
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
    bool dirtyStyle = false;

    media::AudioLoader audio;
    std::shared_ptr<model::Project> proj;
    editor::ProjectWindow projectWindow;
    media::AudioDevice device;
    Player player;
    ResourceManager resourceManager;
    GlobalHotKey hotKey;

    ImFont *loadFont(const char *start, const char *end, float size, bool fontAwesome) const;

    void applyTheme();

    SelectionManager selection;
    Pathie::Path home;
    Pathie::Path iniPath;
    Pathie::Path autoSavePath;
    std::string iniPathStr;

public:
    void error(const std::string &errorMsg) { projectWindow.showError(errorMsg); }

    bool save(std::string filename, bool quiet = false);
    bool load(std::string filename, bool quiet = false);

    Player &getPlayer() { return player; };
    ResourceManager &getResourceManager() { return resourceManager; };
    GlobalHotKey &getHotKey() { return hotKey; };
    SelectionManager &getSelection() { return selection; };
    media::AudioLoader &getAudio() { return audio; };

    void quit();

    void exportIno(std::string filename);

    void exportChunk(std::string filename, model::arduino_number number) const;
    void exportCommons(std::string basic_string);
};

}

#endif //APPLICATION_H
