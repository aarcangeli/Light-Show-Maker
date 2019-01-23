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
        case PROJECT:
            showInputText("Name", manager.proj->name);
            showInputInt("Width", manager.proj->settings.width);
            showInputInt("Height", manager.proj->settings.height);
            editorComponent("Duration (HH:mm:ss:SSS)", manager.proj->canvas.duration);
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

void PropertyPanel::editorComponent(const char *label, time_unit &time) {
    float button_size = ImGui::GetFrameHeight();
    ImGuiStyle &style = ImGui::GetStyle();

    ImGui::BeginGroup();
    ImGui::PushID(label);
    ImGui::PushItemWidth(ImMax(1.0f, ImGui::CalcItemWidth() - (button_size + style.ItemInnerSpacing.x) * 2));

    time_unwrapped u = time_unwrap(time);

    char buf[64];
    ImFormatString(buf, IM_ARRAYSIZE(buf), "%02i:%02i:%02i:%03i", u.hours, u.minutes, u.seconds, u.mills);
    if (ImGui::InputText("", buf, IM_ARRAYSIZE(buf), 0)) {
        parseTime(buf, time);
    }

    ImGui::SameLine(0, style.ItemInnerSpacing.x);
    ImGui::TextUnformatted(label, ImGui::FindRenderedTextEnd(label));

    ImGui::PopItemWidth();
    ImGui::PopID();
    ImGui::EndGroup();
}

bool PropertyPanel::parseTime(const char *_buf, sm::time_unit &time) {
    std::string buf = _buf;

    skipWhitespace(buf);
    if (!buf[0]) return false;

    float p[5]{};
    int scanned = 0;

    while (scanned < 5) {
        if (buf[0] && buf[0] != ':') {
            try {
                size_t idx;
                p[scanned] = std::stof(buf, &idx);
                buf = buf.substr(idx);
                skipWhitespace(buf);
            } catch (std::invalid_argument &arg) {
                return false;
            }
        }
        scanned++;
        if (!buf[0]) break;
        if (buf[0] != ':') return false;
        buf = buf.substr(1);
        skipWhitespace(buf);
    }

    // ensure eof
    if (scanned == 0 || scanned > 4 || buf[0]) return false;

    time_unwrapped u{};
    if (scanned >= 1) {
        float itm = p[scanned - 1];
        u.seconds += static_cast<uint64_t>(itm);
        u.mills += (uint32_t) (itm * 1000) % 1000;
    }
    if (scanned >= 2) {
        float itm = p[scanned - 2];
        u.minutes += static_cast<uint64_t>(itm);
        u.seconds += (uint32_t) (itm * 1000) % 1000;
    }
    if (scanned >= 3) {
        float itm = p[scanned - 3];
        u.hours += static_cast<uint64_t>(itm);
        u.minutes += (uint32_t) (itm * 1000) % 1000;
    }
    if (scanned >= 4) {
        u.mills = u.seconds;
        u.seconds = u.minutes;
        u.minutes = u.hours;
        u.hours = 0;
        float itm = p[scanned - 4];
        u.hours += static_cast<uint64_t>(itm);
        u.minutes += (uint32_t) (itm * 1000) % 1000;
    }

    time_unit res = time_wrap(u);
    if (res < max_time) {
        startChanging();
        time = res;
        return true;
    }

    return false;
}

inline void PropertyPanel::skipWhitespace(std::string &buf) {
    size_t pos = 0;
    while (ImCharIsBlankA(buf[pos]))
        pos++;
    if (pos) buf = buf.substr(pos);
}
