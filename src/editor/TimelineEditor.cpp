#include <Canvas.h>
#include <string>
#include "TimelineEditor.h"
#include <algorithm>
#include <imgui_internal.h>
#include <nfd.h>
#include <Application.h>
#include <IconsFontAwesome4.h>

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
void TimelineEditor::editorOf(const project::Canvas &canvas) {
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

    bool isHovered = IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
//    printf("isHovered: %i\n", isHovered);

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
    delim1 = ImFloor(fontSize * 0.7);

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
        Selectable("Light Group..");
        if (Selectable("Audio File..")) {
            nfdchar_t *outPath = nullptr;
            nfdresult_t result = NFD_OpenDialog("mp3,wav,mp4,avi", app->lastDirectory.c_str(), &outPath);
            if (result == NFD_OKAY) {
                loader.open(std::string(outPath));
                if (!loader.isOpen()) {
                    lastError = "Cannot open '" + std::string(outPath) + "'";
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
}

void TimelineEditor::saveLastDirectory(const nfdchar_t *outPath) const {
    // remove last /
    std::string path = outPath;
    int lastSlash = -1;
    for (int i = 0; path[i]; i++) if (path[i] == '/') lastSlash = i;
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
                    break;
                case StreamType::VIDEO:
                    Button(ICON_FA_VIDEO_CAMERA);
                    break;
                default:
                    Button(ICON_FA_QUESTION);
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

void TimelineEditor::printContent(const project::Canvas &canvas, const ImRect &rect) {
    auto &groups = canvas.groups;
    int groupSize = (int) groups.size();
    time_unit duration = canvas.duration;

    scroll.scrollPaneBegin(
            rect,
            ImVec2(duration * TIME_WIDTH / TIME_UNITS + (rect.Max.x - rect.Min.x) * 0.5f, groupSize * layerHeight + headerBotHeight)
    );

    ImDrawList *drawList = GetWindowDrawList();
    offset = scroll.getOffset();
    scale = scroll.getScale();
    ImFont *font = GetDefaultFont();
    float fontSize = font->FontSize;

    // calculate step
    timeStep = TIME_UNITS;
    while (timeStep * TIME_WIDTH * scale.x / TIME_UNITS * dpi < 8 * dpi) {
        timeStep *= 2;
    }

    firstIndex = ImMax(0, (int) ImFloor(offset.y / layerHeight - headerTopHeight));
    indexMax = ImMin(groupSize, (int) ImCeil((offset.y + rect.Max.y) / layerHeight));

    for (int i = firstIndex / 2 * 2 + 1; i < indexMax; i += 2) {
        ImVec2 pos = rect.Min + ImVec2(0, i * layerHeight - offset.y);
        drawList->AddRectFilled(pos, ImVec2(rect.Max.x, pos.y + layerHeight), OVERLAY_WHITE, 0);
    }

    for (time_unit i = 0; i < duration; i += timeStep * 10) {
        float pos = rect.Min.x + getTimePos(i);
        drawList->AddLine(ImVec2(pos, rect.Min.y), ImVec2(pos, rect.Max.y), COLOR_LINE);
    }

    float zoom = scale.x;

//    int step = 1;
//    while (step * TIME_WIDTH * zoom < 3) {
//        step *= 10;
//    }
//    int stepMini = std::max(step / 10, 1);
//    ImVec2 pos{widgetPos.x + leftSideWidth, widgetPos.y};
//    for (int i = 0; i < 100; i += step) {
//        ImVec2 firstPos = ImFloor(pos + ImVec2(offset.x + i * TIME_WIDTH * zoom * fontSize, 0));
//        drawList->AddText(firstPos + ImVec2(fontSize * 0.2f, 0), textColor, std::to_string(i).c_str());
//    }
//
//    for (int i = 0; i < 100; i += step) {
//        ImVec2 firstPos = ImFloor(pos + ImVec2(offset.x + i * TIME_WIDTH * zoom * fontSize, 0));
//        drawList->AddLine(firstPos, firstPos + ImVec2(0, widgetSize.y), colorLines);
//    }
//
//    for (int i = 0; i < 100; i += stepMini) {
//        ImVec2 firstPos = ImFloor(pos + ImVec2(offset.x + i * TIME_WIDTH * zoom * fontSize, 0));
//        drawList->AddLine(firstPos + ImVec2(0, headerTopHeight) * 0.7, firstPos + ImVec2(0, headerTopHeight), colorLines);
//    }

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

    for (time_unit i = 0; i < duration; i += timeStep * 10) {
        float pos = rect.Min.x + getTimePos(i);
        drawList->AddText(ImVec2(pos, rect.Min.y), textColor, timeLabel(i).c_str());
    }

    EndChild();
    SetCursorPos(oldPos);
}

std::string TimelineEditor::timeLabel(time_unit time) const {
    time_unwrapped unwrapped = time_unwrap(time);
    char buffer[500];
    if (unwrapped.hours) {
        ImFormatString(buffer, sizeof(buffer), "%02llu:%02llu:%02llu:%03llu", unwrapped.hours, unwrapped.minutes, unwrapped.seconds, unwrapped.mills);
    } else {
        ImFormatString(buffer, sizeof(buffer), "%02llu:%02llu:%03llu", unwrapped.minutes, unwrapped.seconds, unwrapped.mills);
    }
    return buffer;
}

void TimelineEditor::printLayerList(const project::Canvas &canvas, ImRect rect) {
    auto &groups = canvas.groups;

    ImVec2 oldPos = GetCursorScreenPos();
    SetCursorScreenPos(rect.Min);

    BeginChild("LayerList", rect.Max - rect.Min, false, ImGuiWindowFlags_NoScrollbar);
    ImDrawList *drawList = GetWindowDrawList();

    drawList->AddRectFilled(ImVec2(rect.Max.x - delim1, rect.Min.y), ImVec2(rect.Max.x, rect.Min.y + indexMax * layerHeight - offset.y),
                            COLOR_BG2);

    for (int i = firstIndex; i < indexMax; i++) {
        ImVec2 pos = rect.Min + ImVec2(0, i * layerHeight - offset.y);
        ImRect labelRect = {pos, ImVec2(rect.Max.x - delim1, pos.y + layerHeight - lineDim)};
        printLayer(groups[i].get(), labelRect);
    }

    EndChild();
    SetCursorPos(oldPos);
}

void TimelineEditor::printLayer(project::LightGroup *group, ImRect rect) {
    ImVec2 oldPos = GetCursorScreenPos();
    SetCursorScreenPos(rect.Min);

    BeginChild(GetCurrentWindow()->GetID(group), rect.Max - rect.Min, false, ImGuiWindowFlags_NoScrollbar);
    ImDrawList *drawList = GetWindowDrawList();

    drawList->AddRectFilled(rect.Min, rect.Max, COLOR_LAYER, 0);

    TextColored(COLOR_TEXT, "%s", group->name.c_str());
    //std::string text = group->name;
    //drawList->AddText(rect.Min, COLOR_TEXT, text.c_str(), text.c_str() + text.size());
    //drawList->AddImage(rect.Min, COLOR_TEXT, text.c_str(), text.c_str() + text.size());

    EndChild();
    SetCursorPos(oldPos);
}

// misc
//drawList->AddRectFilled(ImVec2(0, 0), ImVec2(1000, 1000), 0xff0000ff, 0);
