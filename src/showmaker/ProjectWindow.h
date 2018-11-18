#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include "Editor.h"
#include "memory"

namespace sm {
class Application;

class ProjectWindow {
public:
    explicit ProjectWindow(Application *app);

    // invoked by Application
    void open(std::shared_ptr<project::Project> shared_ptr);
    void close();

    void resize(int width, int height) {
        viewportWidth = width;
        viewportHeight = height;
    }

    void showFrame();

private:
    Application *app;
    std::shared_ptr<project::Project> proj;
    editor::Editor myEditor;
    int viewportWidth = 0, viewportHeight = 0;

    void showMenu();
};

}

#endif //PROJECTWINDOW_H
