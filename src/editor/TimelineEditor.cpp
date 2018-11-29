#include <Canvas.h>
#include <string>
#include "TimelineEditor.h"
#include <algorithm>
#include <imgui_internal.h>
#include <nfd.h>
#include <Application.h>
#include <IconsFontAwesome4.h>

using namespace std;
using namespace sm;
using namespace sm::editor;
using namespace sm::media;
using namespace sm::project;
using namespace ImGui;

TimelineEditor::TimelineEditor() : dragger(this) {
    reset();
}

void TimelineEditor::reset() {
}

/*
 *
 *    +-------------------+----------------------------------------------+      *
 *    |   MEDIA CONTROLS  |   TIMELINE                                   |      | headerTopHeight
 *    +-------------------+----------------------------------------------+      *
 *    | - LAYER 1         | CONTENT                                      |
 *    | - LAYER 2         | KEY1                                         |
 *    | - LAYER 3         | KEY2          KEY3                           |
 *    | - LAYER 4         |                                              |
 *    |                   |                                              |
 *    |                   |                                              |
 *    +-------------------+                                              |      *
 *    | ADD               |                                              |      | headerBotHeight
 *    +-------------------+----------------------------------------------+      *
 *
 */
void TimelineEditor::editorOf(project::Canvas &canvas) {
    this->canvas = &canvas;
    ImGuiStyle &style = GetStyle();
    ImDrawList *drawList = GetWindowDrawList();
    ImGuiIO &io = GetIO();
    int cx = (int) (io.MousePos.x);
    int cy = (int) (io.MousePos.y);
    ImFont *font = GetDefaultFont();
    float fontSize = font->FontSize;

    auto &groups = canvas.groups;
    int groupSize = (int) groups.size();
    time_unit duration = canvas.duration;

    btnActive = ColorConvertFloat4ToU32(style.Colors[ImGuiCol_ButtonActive]);
    btnHover = ColorConvertFloat4ToU32(style.Colors[ImGuiCol_ButtonHovered]);
    const ImU32 background = ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]);
    const ImU32 inactive_color = ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Button]);
    const ImU32 active_color = ColorConvertFloat4ToU32(style.Colors[ImGuiCol_ButtonHovered]);
    const ImU32 line_color = ColorConvertFloat4ToU32(style.Colors[ImGuiCol_ColumnActive]);
    const ImU32 textColor = ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]);
    const ImU32 colorLines = setAlpha(textColor, 0.8);

    ImVec2 widgetPos = ImFloor(GetCursorScreenPos());
    ImVec2 widgetSize = ImFloor(GetContentRegionAvail());

    headerTopHeight = ImFloor(GetFrameHeight() * 1.5f);
    headerBotHeight = headerTopHeight;
    leftSideWidth = fontSize * 20;
    layerHeight = ImFloor(fontSize * 3);
    lineDim = ImFloor(dpi);
    delim1 = ImFloor(fontSize * 0.7f);

    // all region
    ImRect regionRect(widgetPos,
                      widgetPos + widgetSize);
    ImRect mediaControlsRect = ImRect(widgetPos,
                                      widgetPos + ImVec2(leftSideWidth, headerTopHeight));
    ImRect timelineRect = ImRect(widgetPos + ImVec2(leftSideWidth, 0),
                                 widgetPos + ImVec2(widgetSize.x, headerTopHeight));
    ImRect layersRect = ImRect(widgetPos + ImVec2(0, headerTopHeight),
                               widgetPos + ImVec2(leftSideWidth, widgetSize.y - headerBotHeight));
    contentRect = ImRect(widgetPos + ImVec2(leftSideWidth, headerTopHeight),
                         widgetPos + ImVec2(widgetSize.x, widgetSize.y));

    BeginGroup();

    drawList->AddRectFilled(regionRect.Min, regionRect.Max, COLOR_BG, 0);

    drawList->AddRectFilled(widgetPos, ImVec2(widgetSize.x + widgetPos.x, widgetPos.y + headerTopHeight),
                            setAlpha(background, 0.04), 0);

    dragger.update();

    printContent(canvas, contentRect);
    printLayerList(canvas, layersRect);
    printTimeline(canvas, timelineRect);

    SetCursorScreenPos(widgetPos + ImVec2(0, widgetSize.y - headerBotHeight));

    bool openAudioFile = false;
    bool openErrorBox = false;
    if (Button("Add", ImVec2(50, headerBotHeight))) {
        OpenPopup(POPUP_ADD_LAYER);
    }
    if (Button("Add", ImVec2(50, headerBotHeight))) {
        OpenPopup(POPUP_ADD_LAYER);
    }
    if (BeginPopup(POPUP_ADD_LAYER)) {
        if (Selectable("Light Group..")) {
            canvas.makeGroup();
        }
        if (Selectable("Audio File..")) {
            nfdchar_t *outPath = nullptr;
            nfdresult_t result = NFD_OpenDialog("mp3,wav,mp4,avi", gApp->lastDirectory.c_str(), &outPath);
            if (result == NFD_OKAY) {
                loader.open(string(outPath));
                if (!loader.isOpen()) {
                    lastError = "Cannot open '" + string(outPath) + "'";
                    openErrorBox = true;
                } else {
                    openAudioFile = true;
                    saveLastDirectory(outPath);
                }
                free(outPath);
            }
        }
        EndPopup();
    }

    if (openAudioFile) OpenPopup(MODAL_ADD_AUDIO);
    if (openErrorBox) OpenPopup(MODAL_ERROR);
    addAudioModal();
    errorBox();

    EndGroup();
    this->canvas = nullptr;
}

void TimelineEditor::saveLastDirectory(const nfdchar_t *outPath) const {
    // remove last /
    string path = outPath;
    int lastSlash = -1;
    for (int i = 0; path[i]; i++) if (path[i] == '/' || path[i] == '\\') lastSlash = i;
    if (lastSlash >= 0) {
        path[lastSlash] = '\0';
        gApp->lastDirectory = path;
    }
}

ImU32 TimelineEditor::setAlpha(ImU32 color, double alpha) {
    return (color & 0xffffff) + (((uint32_t) (alpha * 0xff) & 0xff) << 27);
}

void TimelineEditor::addAudioModal() {
    if (BeginPopupModal(MODAL_ADD_AUDIO, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        Indent();
        for (auto &stream : loader.streams) {
            switch (stream.type) {
                case StreamType::AUDIO:
                    Button(ICON_FA_HEADPHONES);
                    SameLine();
                    Text("Audio #%i", stream.number);
                    break;
                case StreamType::VIDEO:
                    Button(ICON_FA_VIDEO_CAMERA);
                    SameLine();
                    Text("Video #%i", stream.number);
                    break;
                default:
                    Button(ICON_FA_QUESTION);
                    Text("Unknown #%i", stream.number);
            }
        }
        Unindent();
        Separator();
        if (Button("OK", ImVec2(120, 0))) { CloseCurrentPopup(); }
        SetItemDefaultFocus();
        SameLine();
        if (Button("Cancel", ImVec2(120, 0))) { CloseCurrentPopup(); }
        EndPopup();
    }
}

void TimelineEditor::errorBox() {
    if (BeginPopupModal(MODAL_ERROR, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        Text("Error: %s", lastError.c_str());
        SetCursorScreenPos(GetCursorScreenPos() + ImVec2((GetContentRegionAvail().x - 120) / 2, 0));
        if (Button("OK", ImVec2(120, 0))) { CloseCurrentPopup(); }
        SetItemDefaultFocus();
        EndPopup();
    }
}

float TimelineEditor::getTimePosScreenPos(time_unit time) {
    float off = contentRect.Min.x - offset.x;
    float mult = getTimeScaleX();
    return off + time * mult;
}

bool TimelineEditor::lookUpAtPos(ImVec2 pos, time_unit *time, int *layerIdx) {
    if (!contentRect.Contains(pos)) {
        return false;
    }
    if (time) {
        float off = contentRect.Min.x - offset.x;
        float mult = getTimeScaleX();
        float timeOut = (pos.x - off) / mult;
        *time = static_cast<time_unit>(timeOut);
    }
    if (layerIdx) {
        // ImVec2 pos = contentRect.Min + ImVec2(0, i * layerHeight - offset.y);
        float off = contentRect.Min.y - offset.y;
        float mult = layerHeight;
        float layerIdxOut = (pos.y - off) / mult;
        *layerIdx = static_cast<int>(layerIdxOut);
        if (*layerIdx < 0 || *layerIdx >= canvas->groups.size()) {
            return false;
        }
    }
    return true;
}

float TimelineEditor::getTimeScaleX() const { return TIME_WIDTH / TIME_UNITS * scale.x * dpi; }

bool TimelineEditor::lookMousePos(ImVec2 pos, time_unit *time, int *layerIdx) {
    if (lookUpAtPos(pos, time, layerIdx)) {
        if (time && (snapCursor != GetIO().KeyCtrl)) {
            *time = static_cast<time_unit>(round((float) *time / snapTime) * snapTime);
        }
        return true;
    }
    return false;
}

void TimelineEditor::printContent(project::Canvas &canvas, const ImRect &rect) {
    auto &groups = canvas.groups;
    int groupSize = (int) groups.size();
    time_unit duration = canvas.duration;
    ImGuiIO &io = GetIO();

    scroll.scrollPaneBegin(
            rect,
            ImVec2(duration * TIME_WIDTH / TIME_UNITS * scale.x + (rect.Max.x - rect.Min.x) * 0.5f,
                   groupSize * layerHeight + headerBotHeight)
    );
    bool isHover = IsWindowHovered();

    ImDrawList *drawList = GetWindowDrawList();
    offset = scroll.getOffset();
    scale = scroll.getScale();

    // calculate step
    static time_unit steps[] = {1, 5, 10, 30, 60, 2 * 60, 5 * 60, 0};
    timeStep = steps[0] * TIME_UNITS;
    int count = 1;
    while (timeStep * TIME_WIDTH * scale.x / TIME_UNITS * dpi < 40 * dpi) {
        time_unit num = steps[count++];
        if (!num) break;
        timeStep = num * TIME_UNITS;
    }

    firstIndex = ImMax(0, (int) ImFloor(offset.y / layerHeight - headerTopHeight));
    indexMax = ImMin(groupSize, (int) ImCeil((offset.y + rect.Max.y) / layerHeight));

    for (int i = firstIndex / 2 * 2 + 1; i < indexMax; i += 2) {
        ImVec2 pos = rect.Min + ImVec2(0, i * layerHeight - offset.y);
        drawList->AddRectFilled(pos, ImVec2(rect.Max.x, pos.y + layerHeight), OVERLAY_WHITE, 0);
    }

    for (time_unit i = 0; i < duration; i += timeStep) {
        float pos = getTimePosScreenPos(i);
        drawList->AddLine(ImVec2(pos, rect.Min.y), ImVec2(pos, rect.Max.y), COLOR_LINE);
    }

    time_unit minTime, maxTime;
    lookUpAtPos(rect.Min, &minTime);
    lookUpAtPos(rect.Max, &maxTime);
    ImVec2 mousePos = GetIO().MousePos;

    bool foundKey = false;

    std::shared_ptr<project::KeyPoint> hoverElement;
    if (dragger.isDragging()) {
        hoverElement = dragger.getElement();
        foundKey = true;
    }

    for (int i = firstIndex; i < indexMax; i++) {
        auto &group = canvas.groups[i];
        float screenPosY = rect.Min.y + i * layerHeight - offset.y;
        if (screenPosY + layerHeight < rect.Min.y) continue;
        if (screenPosY > rect.Max.y) continue;

        // find hover element
        float minHandle = COLOR_RESIZE_HANDLE_DIM * dpi / 2;
        if (!foundKey && mousePos.y >= screenPosY && mousePos.y < screenPosY + layerHeight) {
            auto &keys = group->keys;
            for(auto it = keys.rbegin(); it != keys.rend(); it++) {
                auto &k = *it;
                float startOffset = getTimePosScreenPos(k->start);
                float endOffset = getTimePosScreenPos(k->start + k->duration);
                float dim = 0;
                if (k->duration < minHandle) {
                    dim = minHandle;
                }
                if (mousePos.x >= startOffset - dim && mousePos.x < endOffset + dim) {
                    hoverElement = k;
                    foundKey = true;
                    break;
                }
            }
        }

        for (auto &k : group->keys) {
            if (k->start + k->duration < minTime) continue;
            if (k->start > maxTime) continue;
            float startOffset = getTimePosScreenPos(k->start);
            float endOffset = getTimePosScreenPos(k->start + k->duration);
            const ImRect &keyRect = ImRect(startOffset, screenPosY, endOffset, screenPosY + layerHeight);
            bool isKeyHover = hoverElement == k;
            drawKey(group, k, keyRect, isKeyHover);
        }
    }

    if (isHover && io.MouseClicked[0] && !IsKeyDown(GLFW_KEY_SPACE) && !foundKey) {
        time_unit start;
        time_unit duration = TIME_UNITS;
        int32_t layer;
        if (findPlacableKeyPos(GetIO().MouseClickedPos[0], start, duration, layer)) {
            auto &group = canvas.groups[layer];
            gApp->beginCommand("Move/Resize key point");
            group->addKey(start, duration);
            gApp->endCommand();
        }
    }

    scroll.scrollPaneEnd();
}

void
TimelineEditor::drawKey(shared_ptr<LightGroup> group, shared_ptr<KeyPoint> &key, const ImRect &rect, bool isHover) {
    const ImVec2 &min = rect.Min;
    const ImVec2 &max = rect.Max;
    ImDrawList *drawList = GetWindowDrawList();
    ImVec2 mousePos = GetIO().MousePos;

    drawList->AddRectFilled(min, max, isHover ? COLOR_KEY_HOV : COLOR_KEY, COLOR_KEY_RADIUS * dpi);
    drawList->AddRect(min, max, COLOR_KEY_OUTLINE, COLOR_KEY_RADIUS * dpi, ImDrawCornerFlags_All, dpi * 2);

    if (isHover) {
        auto type = KeypointDragger::MOVE;
        float handleDim = COLOR_RESIZE_HANDLE_DIM * dpi;
        if ((!dragger.isDragging() && mousePos.x < min.x + handleDim) ||
            dragger.getType() == KeypointDragger::RESIZE_BEGIN) {
            drawList->AddRectFilled(min, ImVec2(min.x + handleDim, max.y), COLOR_KEY_RESIZE);
            SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            type = KeypointDragger::RESIZE_BEGIN;
        }
        if ((!dragger.isDragging() && mousePos.x > max.x - handleDim) ||
            dragger.getType() == KeypointDragger::RESIZE_END) {
            drawList->AddRectFilled(ImVec2(max.x - handleDim, min.y), max, COLOR_KEY_RESIZE);
            SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            type = KeypointDragger::RESIZE_END;
        }
        if (IsMouseClicked(0)) {
            dragger.startDragging(key, group, type, getTimeScaleX());
        }
    }
}

void TimelineEditor::printTimeline(const project::Canvas &canvas, ImRect rect) {
    const ImU32 textColor = ColorConvertFloat4ToU32(COLOR_TEXT);

    time_unit duration = canvas.duration;
    ImVec2 oldPos = GetCursorScreenPos();
    SetCursorScreenPos(rect.Min);
    BeginChild("Timeline", rect.Max - rect.Min, false, ImGuiWindowFlags_NoScrollbar);
    ImDrawList *drawList = GetWindowDrawList();

    for (time_unit i = 0; i < duration; i += timeStep) {
        float pos = getTimePosScreenPos(i);
        drawList->AddLine(ImVec2(pos, (rect.Min.y + rect.Max.y) / 2), ImVec2(pos, rect.Max.y), COLOR_LINE);
    }

    for (time_unit i = 0; i < duration; i += timeStep) {
        float pos = getTimePosScreenPos(i);
        drawList->AddText(ImVec2(pos, rect.Min.y), textColor, timeLabel(i).c_str());
    }

    EndChild();
    SetCursorPos(oldPos);
}

string TimelineEditor::timeLabel(time_unit time) {
    time_unwrapped unwrapped = time_unwrap(time);
    char buffer[500];
    if (unwrapped.hours) {
        ImFormatString(buffer, sizeof(buffer), "%02llu:%02llu:%02llu", unwrapped.hours, unwrapped.minutes,
                       unwrapped.seconds);
    } else {
        ImFormatString(buffer, sizeof(buffer), "%02llu:%02llu", unwrapped.minutes, unwrapped.seconds);
    }
    return buffer;
}

void TimelineEditor::printLayerList(const project::Canvas &canvas, ImRect rect) {
    auto &groups = canvas.groups;

    ImVec2 oldPos = GetCursorScreenPos();
    SetCursorScreenPos(rect.Min);

    BeginChild("LayerList", rect.Max - rect.Min, false, ImGuiWindowFlags_NoScrollbar);
    isLayerListFocused = IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

    ImDrawList *drawList = GetWindowDrawList();

    drawList->AddRectFilled(ImVec2(rect.Max.x - delim1, rect.Min.y),
                            ImVec2(rect.Max.x, rect.Min.y + indexMax * layerHeight - offset.y),
                            COLOR_BG2);

    for (int i = firstIndex; i < indexMax; i++) {
        ImVec2 pos = rect.Min + ImVec2(0, i * layerHeight - offset.y);
        ImRect labelRect = {pos, ImVec2(rect.Max.x - delim1, pos.y + layerHeight - lineDim)};
        printLayer(groups[i], labelRect);
    }

    EndChild();
    SetCursorPos(oldPos);
}

void TimelineEditor::printLayer(shared_ptr<project::LightGroup> group, ImRect rect) {
    ImGuiIO &io = GetIO();
    ImVec2 oldPos = GetCursorScreenPos();
    SetCursorScreenPos(rect.Min);

    BeginChild(GetCurrentWindow()->GetID(group.get()), rect.Max - rect.Min, false, ImGuiWindowFlags_NoScrollbar);
    bool isHovered = IsWindowHovered(0);
    bool isSelected = selection == group;
    ImDrawList *drawList = GetWindowDrawList();

    if (isHovered && io.MouseClicked[0]) {
        selection = group;
        isSelected = true;
        gApp->layerSelected(group);
    }

    ImU32 color = isSelected ? btnActive : isHovered ? btnHover : COLOR_LAYER;

    drawList->AddRectFilled(rect.Min, rect.Max, color, 0);
    if (isSelected && !isLayerListFocused) {
        drawList->AddRectFilled(rect.Min, rect.Max, COLOR_SEL_UNFOCUS, 0);
    }

    float size = GetFontSize();
    SetCursorScreenPos(rect.Min + ImVec2(10, ((rect.Max.y - rect.Min.y) - size) / 2));
    PushTextWrapPos(rect.Max.x - rect.Min.x - 50);
    TextColored(COLOR_TEXT, "%s", group->name.c_str());
    PopTextWrapPos();

    EndChild();
    SetCursorPos(oldPos);

    if (isLayerListFocused && isSelected && IsKeyPressed(GLFW_KEY_DELETE, false)) {
        deleteTrack(group);
    }
}

void TimelineEditor::deleteTrack(const shared_ptr<project::LightGroup> &group) {
    project::Canvas *canvas = this->canvas;
    gApp->asyncCommand(string("Delete ") + group->name, false, [canvas, group, this]() {
        canvas->deleteGroup(group);
        gApp->layerSelected(nullptr);
        selection.reset();
    });
}

bool TimelineEditor::findPlacableKeyPos(ImVec2 &pos, time_unit &start, time_unit duration, int32_t &layerIdx) {
    if (lookUpAtPos(pos, &start, &layerIdx)) {
        shared_ptr<LightGroup> &layer = canvas->groups[layerIdx];
        size_t idx = layer->findIndex(start);
        if (idx > 0) {
            shared_ptr<KeyPoint> &key = layer->keys[idx - 1];
            if (start < key->start + key->duration / 2) {
                idx--;
            }
        }
        time_unit min = min_time;
        time_unit max = max_time;
        if (idx > 0) {
            shared_ptr<KeyPoint> &key = layer->keys[idx - 1];
            min = key->start + key->duration;
        }
        if (idx < layer->keys.size()) {
            max = layer->keys[idx]->start;
        }
        if (max - min >= duration) {
            if (start < min) start = min;
            if (start + duration > max) start = max - duration;
            return true;
        }
    }
    return false;
}

// misc
//drawList->AddRectFilled(ImVec2(0, 0), ImVec2(1000, 1000), 0xff0000ff, 0);
