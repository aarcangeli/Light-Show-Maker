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
}
