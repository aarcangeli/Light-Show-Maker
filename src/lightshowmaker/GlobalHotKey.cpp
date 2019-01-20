#include "GlobalHotKey.h"
#include "Application.h"

using namespace sm;
using namespace ImGui;

void GlobalHotKey::update() {
    if (IsKeyPressed(GLFW_KEY_ENTER)) {
        gApp->getPlayer().togglePlay();
    }
    if (IsKeyPressed(GLFW_KEY_ESCAPE)) {
        gApp->getPlayer().goBack();
        gApp->getPlayer().pause();
    }
    if (GetIO().KeyCtrl && IsKeyPressed(GLFW_KEY_Z)) {
        if (GetIO().KeyShift) {
            gApp->getHistory().redo();
        } else {
            gApp->getHistory().undo();
        }
    }
    if (GetIO().KeyCtrl && IsKeyPressed(GLFW_KEY_Y)) {
        gApp->getHistory().redo();
    }
}
