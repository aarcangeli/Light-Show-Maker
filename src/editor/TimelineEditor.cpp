#include <Canvas.h>
#include <string>
#include "TimelineEditor.h"
#include <algorithm>
#include <imgui_internal.h>
#include <Application.h>
#include <IconsFontAwesome4.h>
#include <set>

using namespace std;
using namespace sm;
using namespace sm::editor;
using namespace sm::media;
using namespace sm::project;
using namespace ImGui;

TimelineEditor::TimelineEditor() : dragger(this), multiDragger(this) {
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
    auto isFocused = IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
    ImGuiStyle &style = GetStyle();
    ImDrawList *drawList = GetWindowDrawList();
    ImGuiIO &io = GetIO();
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
    layerHeight = ImFloor(fontSize * 2);
    lineDim = ImFloor(dpi);
    delim1 = ImFloor(fontSize * 0.7f);

    // all region
    ImRect regionRect(widgetPos,
                      widgetPos + widgetSize);
    ImRect mediaControlsRect = ImRect(widgetPos,
                                      widgetPos + ImVec2(leftSideWidth, headerTopHeight));
    ImRect timelineRect = ImRect(widgetPos + ImVec2(leftSideWidth, 0),
                                 widgetPos + ImVec2(widgetSize.x, headerTopHeight));
    layersRect = ImRect(widgetPos + ImVec2(0, headerTopHeight),
                        widgetPos + ImVec2(leftSideWidth, widgetSize.y - headerBotHeight));
    contentRect = ImRect(widgetPos + ImVec2(leftSideWidth, headerTopHeight),
                         widgetPos + ImVec2(widgetSize.x, widgetSize.y));

    waveRect = contentRect;
    waveRect.Max.y = waveRect.Min.y + SIZE_WAVE * dpi;
    contentRect.Min.y = waveRect.Max.y;
    layersRect.Min.y = waveRect.Max.y;

    BeginGroup();

    drawList->AddRectFilled(regionRect.Min, regionRect.Max, COLOR_BG, 0);

    drawList->AddRectFilled(widgetPos, ImVec2(widgetSize.x + widgetPos.x, widgetPos.y + headerTopHeight),
                            setAlpha(background, 0.04), 0);

    dragger.update();
    multiDragger.update();

    printMediaControls(mediaControlsRect);
    printContent(canvas, contentRect);
    printLayerList(canvas, layersRect);
    printTimeline(canvas, timelineRect);
    printWave(waveRect);

    SetCursorScreenPos(widgetPos + ImVec2(0, widgetSize.y - headerBotHeight));

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
            string outPath = gApp->getPath("mp3,wav,mp4,avi", false);
            if (!outPath.empty()) {
                loader.open(string(outPath));
                if (!loader.isOpen()) {
                    gApp->error("Cannot open '" + string(outPath) + "'");
                } else {
                    gApp->saveLastDirectory(outPath);
                    canvas.audioFile = outPath;
                    gApp->getPlayer().reloadMedia();
                }
                loader.close();
            }
        }
        EndPopup();
    }

    if (isDraggingPosition) {
        float seek = (GetIO().MousePos.x - getTimeOffsetX()) / getTimeScaleX();
        gApp->getPlayer().seek((time_unit) seek);
        if (!IsMouseDown(0)) {
            isDraggingPosition = false;
        }
    }

    if (isFocused) {
        orderLayers();
    }

    deleteKeypoints();

    EndGroup();
    this->canvas = nullptr;
}

void TimelineEditor::deleteKeypoints() {
    Selection<KeyPoint> &selectedkeypoints = gApp->getSelection().keypoints;
    if (isContentFocused && !selectedkeypoints.empty() && IsKeyPressed(GLFW_KEY_DELETE, false)) {
        gApp->beginCommand(string("Delete" + to_string(selectedkeypoints.size()) + " keys"));
        for (auto &group : canvas->groups) {
            auto &keys = group->keys;
            auto it = keys.begin();
            while(it != keys.end()) {
                if ((*it)->isSelected) {
                    it = keys.erase(it);
                    selectedkeypoints.remove(*it);
                    continue;
                }
                it++;
            }
        }
        gApp->endCommand();
    }
}

ImU32 TimelineEditor::setAlpha(ImU32 color, double alpha) {
    return (color & 0xffffff) + (((uint32_t) (alpha * 0xff) & 0xff) << 27);
}

float TimelineEditor::getTimePosScreenPos(time_unit time) {
    float off = getTimeOffsetX();
    float mult = getTimeScaleX();
    return off + time * mult;
}

float TimelineEditor::getTimeOffsetX() const { return contentRect.Min.x - offset.x; }

bool TimelineEditor::lookUpAtPos(ImVec2 pos, time_unit *time, int *layerIdx) {
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
        if (*layerIdx < 0) *layerIdx = 0;
        if (*layerIdx >= canvas->groups.size()) *layerIdx = canvas->groups.size() - 1;
    }
    return true;
}

bool TimelineEditor::isInsideContent(const ImVec2 &pos) const { return contentRect.Contains(pos); }

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

    ImVec2 oldPos = GetCursorScreenPos();
    SetCursorScreenPos(rect.Min);
    BeginChild("Content", rect.Max - rect.Min, false, ImGuiWindowFlags_NoScrollbar);
    isContentFocused = IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

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
        foundKey = true;
    }

    for (int i = firstIndex; i < indexMax; i++) {
        auto &group = canvas.groups[i];
        auto &keys = group->keys;
        float screenPosY = rect.Min.y + i * layerHeight - offset.y;
        if (screenPosY + layerHeight < rect.Min.y) continue;
        if (screenPosY > rect.Max.y) continue;

        // find hover element
        float minHandle = COLOR_RESIZE_HANDLE_DIM * dpi / 2;
        if (isHover && !foundKey && mousePos.y >= screenPosY && mousePos.y < screenPosY + layerHeight) {
            for (auto it = keys.rbegin(); it != keys.rend(); it++) {
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

        for (auto &k : keys) {
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

    float pos = getTimePosScreenPos(gApp->getPlayer().playerPosition());
    drawList->AddLine(ImVec2(pos, rect.Min.y), ImVec2(pos, rect.Max.y), COLOR_CURSOR);

    scroll.scrollPaneEnd();

    EndChild();
    SetCursorPos(oldPos);
}

void TimelineEditor::drawKey(shared_ptr<Layer> group, shared_ptr<KeyPoint> &key, const ImRect &rect, bool isHover) {
    const ImVec2 &min = rect.Min;
    const ImVec2 &max = rect.Max;

    ImDrawList *drawList = GetWindowDrawList();
    ImGuiIO &io = GetIO();
    ImVec2 mousePos = io.MousePos;
    float timeScaleX = getTimeScaleX();

    drawList->AddRectFilled(min, max, key->isSelected ? COLOR_KEY_SEL : isHover ? COLOR_KEY_HOV : COLOR_KEY, COLOR_KEY_RADIUS * dpi);

    float easing1 = timeScaleX * key->fadeStart.duration;
    float easing2 = timeScaleX * (key->duration - key->fadeEnd.duration);

    // draw path
    float cur = 0;
    float lineSize = dpi * 2;
    float sizeY = max.y - min.y - lineSize * 2;
    float sizeX = max.x - min.x;

    drawList->PathClear();
    while (cur < easing1 + CURVE_RESOLUTION * 4 and cur < sizeX) {
        time_unit now = (time_unit) (cur / timeScaleX);
        float i = key->computeEasing(now);
        drawList->PathLineTo(ImVec2(min.x + cur, max.y - lineSize - i * sizeY));
        cur += CURVE_RESOLUTION;
    }
    if (cur < easing2 - CURVE_RESOLUTION * 4) cur = easing2 - CURVE_RESOLUTION * 4;
    while (cur < sizeX) {
        time_unit now = (time_unit) (cur / timeScaleX);
        float i = key->computeEasing(now);
        drawList->PathLineTo(ImVec2(min.x + cur, max.y - lineSize - i * sizeY));
        cur += CURVE_RESOLUTION;
    }
    drawList->PathLineTo(ImVec2(max.x, max.y - lineSize - key->computeEasing(key->duration) * sizeY));
    drawList->PathStroke(COLOR_KEY_GRAPH, false, lineSize);

    drawList->AddRect(min, max, COLOR_KEY_OUTLINE, COLOR_KEY_RADIUS * dpi, ImDrawCornerFlags_All, dpi * 2);

    if (isHover) {
        DragType type = DRAG_MOVE;
        float handleDim = COLOR_RESIZE_HANDLE_DIM * dpi;
        if ((!dragger.isDragging() && mousePos.x < min.x + handleDim) ||
            dragger.getType() == DRAG_RESIZE_BEGIN) {
            drawList->AddRectFilled(min, ImVec2(min.x + handleDim, max.y), COLOR_KEY_RESIZE);
            SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            type = DRAG_RESIZE_BEGIN;
        }
        if ((!dragger.isDragging() && mousePos.x > max.x - handleDim) ||
            dragger.getType() == DRAG_RESIZE_END) {
            drawList->AddRectFilled(ImVec2(max.x - handleDim, min.y), max, COLOR_KEY_RESIZE);
            SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            type = DRAG_RESIZE_END;
        }
        if (IsMouseClicked(0) && !IsKeyDown(GLFW_KEY_SPACE)) {
            Selection<KeyPoint> &keypoints = gApp->getSelection().keypoints;
            if (keypoints.size() <= 1 || io.KeyCtrl) {
                if (io.KeyCtrl) {
                    keypoints.toggle(key);
                } else {
                    keypoints.set(key);
                }
                if (io.KeyAlt) {
                    gApp->asyncCommand("Duplicate", false, [=]() {
                        shared_ptr<KeyPoint> duplicated = std::make_shared<KeyPoint>(*key);
                        group->addKey(duplicated);
                        dragger.startDragging(duplicated, group, type, timeScaleX);
                        gApp->getSelection().keypoints.set(duplicated);
                    });
                } else {
                    dragger.startDragging(key, group, type, timeScaleX);
                }
            } else {
                if (!key->isSelected) {
                    keypoints.set(key);
                } else {
                    keypoints.remove(key);
                    keypoints.add(key);
                    Canvas *pCanvas = canvas;
                    auto vector = keypoints.getVector();
                    if (io.KeyAlt) {
                        gApp->asyncCommand("Duplicate bulk", false, [=, &keypoints]() {
                            keypoints.reset();
                            for(auto &k : vector) {
                                shared_ptr<Layer> owner = pCanvas->findGroupWith(k);
                                if (owner) {
                                    shared_ptr<KeyPoint> duplicated = std::make_shared<KeyPoint>(*k);
                                    owner->addKey(duplicated);
                                    keypoints.add(duplicated);
                                }
                            }
                            multiDragger.startDragging(keypoints.getVector(), pCanvas, type, timeScaleX);
                        });
                    } else {
                        multiDragger.startDragging(vector, pCanvas, type, timeScaleX);
                    }
                }
            }
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

    float offset = -1;
    for (time_unit i = 0; i < duration; i += timeStep) {
        float pos = getTimePosScreenPos(i);
        const string &text = timeLabel(i, false);
        const ImVec2 &vec2 = CalcTextSize(text.c_str());
        if (offset < pos) {
            drawList->AddText(ImVec2(pos, rect.Min.y), textColor, text.c_str());
            offset = pos + vec2.x;
        }
    }

    time_unit playerPosition = gApp->getPlayer().playerPosition();
    float pos = getTimePosScreenPos(playerPosition);
    float mid = (rect.Max.y - rect.Min.y) / 2;
    drawList->AddLine(ImVec2(pos, rect.Min.y + mid), ImVec2(pos, rect.Max.y), COLOR_CURSOR);
    drawList->AddText(ImVec2(pos, rect.Min.y + mid), COLOR_CURSOR, timeLabel(playerPosition, true).c_str());

    if (IsMouseDown(0) && IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) {
        isDraggingPosition = true;
    }

    EndChild();
    SetCursorPos(oldPos);
}

string TimelineEditor::timeLabel(time_unit time, bool withMills) {
    time_unwrapped unwrapped = time_unwrap(time);
    char buffer[500];
    if (unwrapped.hours) {
        ImFormatString(buffer, sizeof(buffer), "%02i:%02i:%02i", unwrapped.hours, unwrapped.minutes, unwrapped.seconds);
    } else {
        ImFormatString(buffer, sizeof(buffer), "%02i:%02i", unwrapped.minutes, unwrapped.seconds);
    }
    if (withMills) {
        ImFormatString(buffer, sizeof(buffer), "%s:%03i", buffer, unwrapped.mills);
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

void TimelineEditor::printLayer(shared_ptr<project::Layer> group, ImRect rect) {
    ImGuiIO &io = GetIO();
    ImVec2 oldPos = GetCursorScreenPos();
    SetCursorScreenPos(rect.Min);

    BeginChild(GetCurrentWindow()->GetID(group.get()), rect.Max - rect.Min, false, ImGuiWindowFlags_NoScrollbar);
    bool isHovered = IsWindowHovered(0);
    bool isSelected = group->isSelected;
    ImDrawList *drawList = GetWindowDrawList();

    if (isHovered && io.MouseClicked[0]) {
        isSelected = true;
        auto &layers = gApp->getSelection().layers;
        if (io.KeyCtrl) {
            layers.toggle(group);
        } else {
            layers.set(group);
        }
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

void TimelineEditor::deleteTrack(const shared_ptr<project::Layer> &group) {
    project::Canvas *canvas = this->canvas;
    gApp->asyncCommand(string("Delete ") + group->name, false, [canvas, group, this]() {
        canvas->deleteGroup(group);
        gApp->getSelection().layers.remove(group);
    });
}

bool TimelineEditor::findPlacableKeyPos(ImVec2 &pos, time_unit &start, time_unit duration, int32_t &layerIdx) {
    if (lookUpAtPos(pos, &start, &layerIdx)) {
        shared_ptr<Layer> &layer = canvas->groups[layerIdx];
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

void TimelineEditor::printMediaControls(ImRect rect) {
    ImVec2 oldPos = GetCursorScreenPos();
    SetCursorScreenPos(rect.Min);
    BeginChild("MediaControls", rect.Max - rect.Min, false, ImGuiWindowFlags_NoScrollbar);

    float rectMaxY = rect.Max.y - rect.Min.y;
    ImVec2 size(rectMaxY, rectMaxY);

    Player &player = gApp->getPlayer();
    if (player.playing()) {
        if (Button(ICON_FA_PAUSE, size)) {
            player.pause();
        }
    } else {
        if (Button(ICON_FA_PLAY, size)) {
            player.play();
        }
    }

    EndChild();
    SetCursorScreenPos(oldPos);
}

time_unit TimelineEditor::moveSnapped(time_unit input, TimelineEditor::KeyChecker checker) {
    time_unit best = -1;
    time_unit bestDest = 0;
    findBestSnap(input, best, bestDest, checker);
    snapItem(gApp->getPlayer().playerPosition(), input, nullptr, best, bestDest, checker);
    if (best >= 0) {
        return bestDest;
    }
    return input;
}

bool TimelineEditor::findBestSnap(time_unit input, time_unit &best, time_unit &bestDest,
                                  const TimelineEditor::KeyChecker &checker) const {
    for (auto &group : canvas->groups) {
        auto &keys = group->keys;
        int index = (int) group->findIndex(input);
        int size = (int) keys.size();
        for (int i = index - 3; i <= index + 3; i++) {
            if (i >= 0 && i < size) {
                auto key = keys[i];
                snapItem(key->start, input, key, best, bestDest, checker);
                snapItem(key->start + key->duration, input, key, best, bestDest, checker);
            }
        }
    }
    return best >= 0;
}

void TimelineEditor::snapItem(time_unit dest, time_unit input, const shared_ptr<KeyPoint> &key, time_unit &best,
                              time_unit &bestDest, KeyChecker checker) const {
    time_unit diff = abs(dest - input);
    if ((best < 0 || diff < best) && (!key || checker(dest, key))) {
        best = diff;
        bestDest = dest;
    }
}

void TimelineEditor::printWave(ImRect rect) {
    vector<int16_t> &samples = gApp->getAudio().samples;
    int sampleRate = gApp->getAudio().sampleRate;
    size_t max = samples.size();

    ImDrawList *drawList = GetWindowDrawList();

    float spaceX = rect.Max.x - rect.Min.x;
    float multY = (rect.Max.y - rect.Min.y) / 2;
    float startX = rect.Min.x;
    float startY = rect.Min.y + multY;
    ImVec2 cursor;

    float timeOffsetX = getTimeOffsetX();
    float timeScaleX = getTimeScaleX();

    drawList->PathClear();
    for (int i = 0; i < spaceX; i++) {
        cursor.x = startX + i;
        size_t samplePos = (size_t) ((cursor.x - timeOffsetX) / timeScaleX / TIME_UNITS * sampleRate);
        if (samplePos < 0) continue;
        if (samplePos >= max) break;
        float alpha = (float) samples[samplePos] / 0x7fff;
        cursor.y = startY - alpha * multY;
        drawList->PathLineTo(cursor);
    }

    drawList->PathStroke(COLOR_LINE, false);
}

void TimelineEditor::orderLayers() {
    auto &selection = gApp->getSelection().layers;
    if (selection.empty()) {
        return;
    }
    shared_ptr<project::Layer> selected = selection[0];

    auto &array = canvas->groups;
    auto it = array.begin();
    while (it != array.end()) {
        shared_ptr<project::Layer> el = *it;
        if (el == selected) {
            if (IsKeyPressed(GLFW_KEY_PAGE_UP) && it != array.begin()) {
                it = array.erase(it);
                it--;
                it = array.insert(it, el);
                return;
            }
            if (IsKeyPressed(GLFW_KEY_PAGE_DOWN) && it != array.end() - 1) {
                it = array.erase(it);
                it = array.insert(it + 1, el);
                return;
            }
            if (IsKeyPressed(GLFW_KEY_INSERT) && GetIO().KeyAlt) {
                auto layer = std::make_shared<project::Layer>();
                layer->name = el->name;
                layer->number = el->number;
                layer->identifier = el->identifier;
                array.insert(it + 1, layer);
                return;
            }
        }
        it++;
    }
}

// misc
//drawList->AddRectFilled(ImVec2(0, 0), ImVec2(1000, 1000), 0xff0000ff, 0);
