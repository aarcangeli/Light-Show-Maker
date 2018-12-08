#include <algorithm>
#include "Canvas.h"

using namespace sm::project;

Canvas::Canvas(Project *project) : project(project) {
}

std::shared_ptr<Layer> Canvas::makeGroup() {
    groups.emplace_back(new Layer());
    std::shared_ptr<Layer> g = groups.back();
    g->name = "Track #" + std::to_string(groups.size());
    return g;
}

void Canvas::deleteGroup(const std::shared_ptr<Layer> &group) {
    auto position = std::find(groups.begin(), groups.end(), group);
    if (position != groups.end())
        groups.erase(position);
}
