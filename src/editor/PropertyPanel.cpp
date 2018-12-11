#include "PropertyPanel.h"

#include "imgui.h"
#include "imgui_internal.h"

using namespace sm;
using namespace sm::editor;
using namespace ImGui;

// todo: every edit should have a command

void PropertyPanel::showProperties(SelectionManager &manager) {
    switch (manager.lastSelection) {
        case NOTHING:
            break;
        case LAYER:
            if (manager.layers.size() == 1) {
                showPropertiesOf(manager.layers[0]);
            }
            break;
        case DECORATION:
            if (manager.decorations.size() == 1) {
                showPropertiesOf(manager.decorations[0]);
            }
            break;
        case KEYPOINT:
            if (manager.keypoints.size() == 1) {
                showPropertiesOf(manager.keypoints[0]);
            } else {
                multiKeypointEditor(manager);
            }
            if (Button("Paste Fade Start")) {
                for (auto &key : manager.keypoints) {
                    key->fadeStart = fade;
                }
            }
            SameLine();
            if (Button("Paste Fade End")) {
                for (auto &key : manager.keypoints) {
                    key->fadeEnd = fade;
                }
            }
            break;
    }
}

void PropertyPanel::showPropertiesOf(std::shared_ptr<project::Decoration> &layer) {
    if (layer->type == project::LIGHT) {
        SliderFloat("size", &layer->size, 5, 50, "%.2f");
        ImColor c(layer->color);
        float values[] = {c.Value.x, c.Value.y, c.Value.z};
        if (ImGui::ColorEdit3("Color", values)) {
            c.Value.x = values[0];
            c.Value.y = values[1];
            c.Value.z = values[2];
            layer->color = ImU32(c);
        }

    }
    if (layer->type == project::IMAGE) {
        if (layer->ratio < 0) {
            layer->ratio = layer->width / layer->height;
        }
        if (SliderFloat("width", &layer->width, 1, 10000, "%.2f")) {
            layer->height = layer->width / layer->ratio;
        }
        if (SliderFloat("height", &layer->height, 1, 10000, "%.2f")) {
            layer->width = layer->height * layer->ratio;
        }
    }
}

void PropertyPanel::showPropertiesOf(std::shared_ptr<project::Layer> &layer) {
    char buffer[200];
    strcpy_s(buffer, sizeof(buffer), layer->name.c_str());
    if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
        layer->name = buffer;
    }
    InputInt("Arduino Number", &layer->number);
    strcpy_s(buffer, sizeof(buffer), layer->identifier.c_str());
    if (ImGui::InputText("Identifier", buffer, sizeof(buffer))) {
        layer->identifier = buffer;
    }
}

void PropertyPanel::showPropertiesOf(std::shared_ptr<project::KeyPoint> &keypoint) {
    InputInt("Start", &keypoint->start);
    InputInt("Duration", &keypoint->duration);
    InputFloat("Max Weight", &keypoint->maxWeight);
    showPropertiesOf(keypoint->fadeStart, "Fade start", keypoint->duration - keypoint->fadeEnd.duration);
    showPropertiesOf(keypoint->fadeEnd, "Fade end", keypoint->duration - keypoint->fadeStart.duration);
    if (Button("Copy Fade Start")) {
        fade = keypoint->fadeStart;
    }
    SameLine();
    if (Button("Copy Fade End")) {
        fade = keypoint->fadeEnd;
    }
}

void PropertyPanel::showPropertiesOf(project::Fade &fade, const char *name, time_unit max) {
    static const char *boxes[] = {"Linear", "Exponential", "Sin", "SinDouble"};
    int count = IM_ARRAYSIZE(boxes);

    PushID(name);
    if (CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen)) {
        Combo("Type", (int *) &fade.type, boxes, count);
        SliderInt("Time", &fade.duration, 0, max);

        if (fade.type == project::EXPONENTIAL) {
            InputFloat("Exponent", &fade.exponent);
        }
    }
    PopID();
}

void PropertyPanel::multiKeypointEditor(SelectionManager &manager) {
    if (manager.keypoints.empty()) return;

    bulkEditor("Duration", manager,
               [](KeyRef key) -> int & { return (int &) key->duration; },
               [](KeyRef key) -> int { return TIME_UNITS * 2; });

    bulkEditor("Start", manager,
               [](KeyRef key) -> int & { return (int &) key->fadeStart.duration; },
               [](KeyRef key) -> int { return key->duration - key->fadeEnd.duration; });

    bulkEditor("End", manager,
               [](KeyRef key) -> int & { return (int &) key->fadeEnd.duration; },
               [](KeyRef key) -> int { return key->duration - key->fadeStart.duration; });
}

void PropertyPanel::bulkEditor(const char *name, SelectionManager &manager,
                               std::function<int & (KeyRef)> getter,
                               std::function<int(KeyRef)> maxGetter) {
    int item = getter(manager.keypoints[0]);
    int max = 0;
    for (auto &key : manager.keypoints) {
        max = std::max(max, maxGetter(key));
    }
    if (SliderInt(name, &item, 0, max)) {
        for (auto &key : manager.keypoints) {
            getter(key) = std::min(item, maxGetter(key));
        }
    }
}
