#include "PropertyPanel.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "Application.h"

using namespace sm;
using namespace sm::editor;

void PropertyPanel::showProperties(SelectionManager &manager) {
    somethingChanged = false;
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
            if (!manager.keypoints.empty()) {
                if (ImGui::Button("Paste Fade Start")) {
                    startChanging("Paste Fade Start");
                    for (auto &key : manager.keypoints) {
                        key->fadeStart = fade;
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Paste Fade End")) {
                    startChanging("Paste Fade End");
                    for (auto &key : manager.keypoints) {
                        key->fadeEnd = fade;
                    }
                }
            }
            break;
    }
    if (somethingChanged) {
        gApp->endCommand();
    }
}

void PropertyPanel::showPropertiesOf(std::shared_ptr<model::Decoration> &layer) {
    if (layer->type == model::LIGHT) {
        showSlider("size", layer->size, 5, 50, "%.2f");
        showColorEdit3("Color", layer->color);
    }
    if (layer->type == model::IMAGE) {
        if (layer->ratio < 0) {
            layer->ratio = layer->width / layer->height;
        }
        if (showSlider("width", layer->width, 1, 10000, "%.2f")) {
            layer->height = layer->width / layer->ratio;
        }
        if (showSlider("height", layer->height, 1, 10000, "%.2f")) {
            layer->width = layer->height * layer->ratio;
        }
    }
}

void PropertyPanel::showPropertiesOf(std::shared_ptr<model::Layer> &layer) {
    showInputText("Name", layer->name);
    showInputInt("Arduino Number", layer->number);
    showInputText("Identifier", layer->identifier);
}

void PropertyPanel::showPropertiesOf(std::shared_ptr<model::KeyPoint> &keypoint) {
    showInputInt("Start", keypoint->start);
    showInputInt("Duration", keypoint->duration);
    showInputFloat("Max Weight", keypoint->maxWeight);
    showPropertiesOf(keypoint->fadeStart, "Fade start", keypoint->duration - keypoint->fadeEnd.duration);
    showPropertiesOf(keypoint->fadeEnd, "Fade end", keypoint->duration - keypoint->fadeStart.duration);
    if (ImGui::Button("Copy Fade Start")) {
        fade = keypoint->fadeStart;
    }
    ImGui::SameLine();
    if (ImGui::Button("Copy Fade End")) {
        fade = keypoint->fadeEnd;
    }
}

void PropertyPanel::showPropertiesOf(model::Fade &fade, const char *name, time_unit max) {
    static const char *boxes[] = {"Linear", "Exponential", "Sin", "SinDouble"};
    int count = IM_ARRAYSIZE(boxes);

    ImGui::PushID(name);
    if (ImGui::CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen)) {
        showCombo("Type", (int *) &fade.type, boxes, count);
        showSlider("Time", fade.duration, 0, max);

        if (fade.type == model::EXPONENTIAL) {
            showInputFloat("Exponent", fade.exponent);
        }
    }
    ImGui::PopID();
}

void PropertyPanel::multiKeypointEditor(SelectionManager &manager) {
    if (manager.keypoints.empty()) return;

    bulkEditor<float>("Max Weight", manager,
                      [](KeyRef key) -> float & { return (float &) key->maxWeight; },
                      [](KeyRef key) -> float { return 1; });

    bulkEditor<int>("Duration", manager,
                    [](KeyRef key) -> int & { return (int &) key->duration; },
                    [](KeyRef key) -> int { return TIME_UNITS * 2; });

    bulkEditor<int>("Start", manager,
                    [](KeyRef key) -> int & { return (int &) key->fadeStart.duration; },
                    [](KeyRef key) -> int { return key->duration - key->fadeEnd.duration; });

    bulkEditor<int>("End", manager,
                    [](KeyRef key) -> int & { return (int &) key->fadeEnd.duration; },
                    [](KeyRef key) -> int { return key->duration - key->fadeStart.duration; });
}


template<typename T>
void PropertyPanel::bulkEditor(const char *name, SelectionManager &manager,
                               std::function<T & (KeyRef)> getter,
                               std::function<T(KeyRef)> maxGetter) {
    T item = getter(manager.keypoints[0]);
    T max = 0;
    for (auto &key : manager.keypoints) {
        max = std::max(max, maxGetter(key));
    }
    if (showSlider(name, item, 0, max)) {
        for (auto &key : manager.keypoints) {
            getter(key) = std::min(item, maxGetter(key));
        }
    }
}

bool PropertyPanel::showSlider(const char *name, int &item, int min, int max) {
    int currentValue = item;
    if (ImGui::SliderInt(name, &currentValue, min, max)) {
        startChanging();
        item = currentValue;
        return true;
    }
    return false;
}

bool PropertyPanel::showSlider(const char *name, float &item, float min, float max, const char* format) {
    float currentValue = item;
    if (ImGui::SliderFloat(name, &currentValue, min, max, format)) {
        startChanging();
        item = currentValue;
        return true;
    }
    return false;
}

bool PropertyPanel::showInputInt(const char *name, int &item) {
    int currentValue = item;
    if (ImGui::InputInt(name, &currentValue)) {
        startChanging();
        item = currentValue;
        return true;
    }
    return false;
}

bool PropertyPanel::showInputFloat(const char *name, float &item) {
    float currentValue = item;
    if (ImGui::InputFloat(name, &currentValue)) {
        startChanging();
        item = currentValue;
        return true;
    }
    return false;
}

bool PropertyPanel::showCombo(const char *name, int *item, const char **items, int items_count) {
    int currentValue = *item;
    if (ImGui::Combo(name, &currentValue, items, items_count)) {
        startChanging();
        *item = currentValue;
        return true;
    }
    return false;
}

bool PropertyPanel::showInputText(const char *name, std::string &value) {
    static char buffer[200];
    strcpy_s(buffer, sizeof(buffer), value.c_str());
    if (ImGui::InputText(name, buffer, sizeof(buffer))) {
        startChanging();
        value = buffer;
        return true;
    }
    return false;
}

bool PropertyPanel::showColorEdit3(const char *name, uint32_t &value) {
    ImColor c(value);
    float values[] = {c.Value.x, c.Value.y, c.Value.z};
    if (ImGui::ColorEdit3("Color", values)) {
        c.Value.x = values[0];
        c.Value.y = values[1];
        c.Value.z = values[2];
        value = ImU32(c);
        return true;
    }
    return false;
}

void PropertyPanel::startChanging(const char *name) {
    if (!somethingChanged) {
        bool res = gApp->beginCommand(name, true);
        assert(res); // todo: when it is false??
        somethingChanged = true;
    }
}
