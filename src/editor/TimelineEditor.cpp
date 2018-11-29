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
using namespace ImGui;

TimelineEditor::TimelineEditor(Application *app) : app(app) {
    assert(app);
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
    ImRect regionRect(widgetPos, widgetPos + widgetSize);
    ImRect mediaControlsRect = ImRect(widgetPos, widgetPos + ImVec2(leftSideWidth, headerTopHeight));
    ImRect timelineRect = ImRect(widgetPos + ImVec2(leftSideWidth, 0), widgetPos + ImVec2(widgetSize.x, headerTopHeight));
    ImRect layersRect = ImRect(widgetPos + ImVec2(0, headerTopHeight), widgetPos + ImVec2(leftSideWidth, widgetSize.y - headerBotHeight));
    ImRect keysRect = ImRect(widgetPos + ImVec2(leftSideWidth, headerTopHeight), widgetPos + ImVec2(widgetSize.x, widgetSize.y));

    BeginGroup();

    drawList->AddRectFilled(regionRect.Min, regionRect.Max, COLOR_BG, 0);

    drawList->AddRectFilled(widgetPos, ImVec2(widgetSize.x + widgetPos.x, widgetPos.y + headerTopHeight), setAlpha(background, 0.04), 0);

    printContent(canvas, keysRect);
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
            nfdresult_t result = NFD_OpenDialog("mp3,wav,mp4,avi", app->lastDirectory.c_str(), &outPath);
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
        app->lastDirectory = path;
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

float TimelineEditor::getTimePos(time_unit time) {
    return time * TIME_WIDTH / TIME_UNITS * scale.x * dpi - offset.x;
}

void TimelineEditor::printContent(project::Canvas &canvas, const ImRect &rect) {
    auto &groups = canvas.groups;
    int groupSize = (int) groups.size();
    time_unit duration = canvas.duration;

    scroll.scrollPaneBegin(
            rect,
            ImVec2(duration * TIME_WIDTH / TIME_UNITS * scale.x + (rect.Max.x - rect.Min.x) * 0.5f,
                   groupSize * layerHeight + headerBotHeight)
    );

    ImDrawList *drawList = GetWindowDrawList();
    offset = scroll.getOffset();
    scale = scroll.getScale();
    ImFont *font = GetDefaultFont();

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
        float pos = rect.Min.x + getTimePos(i);
        drawList->AddLine(ImVec2(pos, rect.Min.y), ImVec2(pos, rect.Max.y), COLOR_LINE);
    }

    scroll.scrollPaneEnd();
}

void TimelineEditor::printTimeline(const project::Canvas &canvas, ImRect rect) {
    const ImU32 textColor = ColorConvertFloat4ToU32(COLOR_TEXT);

    time_unit duration = canvas.duration;
    ImVec2 oldPos = GetCursorScreenPos();
    SetCursorScreenPos(rect.Min);
    BeginChild("Timeline", rect.Max - rect.Min, false, ImGuiWindowFlags_NoScrollbar);
    ImDrawList *drawList = GetWindowDrawList();

    for (time_unit i = 0; i < duration; i += timeStep) {
        float pos = rect.Min.x + getTimePos(i);
        drawList->AddLine(ImVec2(pos, (rect.Min.y + rect.Max.y) / 2), ImVec2(pos, rect.Max.y), COLOR_LINE);
    }

    for (time_unit i = 0; i < duration; i += timeStep) {
        float pos = rect.Min.x + getTimePos(i);
        drawList->AddText(ImVec2(pos, rect.Min.y), textColor, timeLabel(i).c_str());
    }

    EndChild();
    SetCursorPos(oldPos);
}

string TimelineEditor::timeLabel(time_unit time) {
    time_unwrapped unwrapped = time_unwrap(time);
    char buffer[500];
    if (unwrapped.hours) {
        ImFormatString(buffer, sizeof(buffer), "%02llu:%02llu:%02llu", unwrapped.hours, unwrapped.minutes, unwrapped.seconds);
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
        app->layerSelected(group);
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
    Application *app = this->app;
    app->command(string("Delete ") + group->name, [canvas, group, app, this]() {
        canvas->deleteGroup(group);
        app->layerSelected(nullptr);
        selection.reset();
    });
}

// misc
//drawList->AddRectFilled(ImVec2(0, 0), ImVec2(1000, 1000), 0xff0000ff, 0);
