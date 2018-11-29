#include <algorithm>
#include "Canvas.h"

using namespace sm::project;

Canvas::Canvas(Project *project) : project(project) {
}

std::shared_ptr<LightGroup> Canvas::makeGroup() {
    groups.emplace_back(new LightGroup());
    std::shared_ptr<LightGroup> g = groups.back();
    g->name = "Track #" + std::to_string(groups.size());
    return g;
}

void Canvas::deleteGroup(const std::shared_ptr<LightGroup> &group) {
    auto position = std::find(groups.begin(), groups.end(), group);
    if (position != groups.end())
        groups.erase(position);
}
