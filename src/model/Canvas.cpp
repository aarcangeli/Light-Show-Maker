#include "Canvas.h"

using namespace sm::project;

Canvas::Canvas(Project *project) : project(project) {
}

std::shared_ptr<LightGroup> Canvas::makeGroup() {
    groups.emplace_back(new LightGroup(this));
    std::shared_ptr<LightGroup> g = groups.back();
    g->name = "Track #" + std::to_string(groups.size());
    return g;
}
