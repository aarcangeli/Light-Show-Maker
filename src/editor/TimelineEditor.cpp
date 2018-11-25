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
    zoom = 1;
    zoomTarget = 1;
}

void TimelineEditor::editorOf(project::Canvas canvas) {
    ImGuiStyle &style = GetStyle();
    ImDrawList *draw_list = GetWindowDrawList();
    ImGuiIO &io = GetIO();
    int cx = (int) (io.MousePos.x);
    int cy = (int) (io.MousePos.y);
    ImFont *font = GetDefaultFont();
    float fontSize = font->FontSize;

    bool isHovered = IsWindowHovered();

    const ImU32 background = ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]);
    const ImU32 inactive_color = ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Button]);
    const ImU32 active_color = ColorConvertFloat4ToU32(style.Colors[ImGuiCol_ButtonHovered]);
    const ImU32 line_color = ColorConvertFloat4ToU32(style.Colors[ImGuiCol_ColumnActive]);
    const ImU32 textColor = ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]);
    const ImU32 colorLines = setAlpha(textColor, 0.8);

    int headerHeight = GetFrameHeight();
    int headerBottom = headerHeight;
    int leftSideWidth = fontSize * 20;

    time_unit duration = canvas.duration;

    BeginGroup();

    ImVec2 canvas_pos = GetCursorScreenPos();
    ImVec2 canvas_size = GetContentRegionAvail();

    ImRect regionRect(canvas_pos, canvas_pos + canvas_size);
    if (isHovered && regionRect.Contains(io.MousePos)) {
        if (io.MouseWheel != 0) {
            zoomTarget *= pow(1.1, io.MouseWheel);
            zoomTarget = ImClamp(zoomTarget, 0.076278f, 49.785397f);
        }
    }
    zoom = ImLerp(zoom, zoomTarget, 0.33f);

    bool hasScrollBar = false;

    draw_list->AddRectFilled(regionRect.Min + ImVec2(0, headerHeight),
                             ImVec2(canvas_pos.x + leftSideWidth, regionRect.Max.y - headerBottom),
                             setAlpha(background, 0.03), 0);

    draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + headerHeight),
                             setAlpha(background, 0.04), 0);

    // header times
    int step = 1;
    while (step * TIME_WIDTH * zoom < 3) {
        step *= 10;
    }
    int stepMini = std::max(step / 10, 1);
    ImVec2 pos{canvas_pos.x + leftSideWidth, canvas_pos.y};
    for (int i = 0; i < 100; i += step) {
        ImVec2 firstPos = ImFloor(pos + ImVec2(i * TIME_WIDTH * zoom * fontSize, 0));
        draw_list->AddText(firstPos + ImVec2(fontSize * 0.2f, 0), textColor, std::to_string(i).c_str());
    }

    for (int i = 0; i < 100; i += step) {
        ImVec2 firstPos = ImFloor(pos + ImVec2(i * TIME_WIDTH * zoom * fontSize, 0));
        draw_list->AddLine(firstPos, firstPos + ImVec2(0, canvas_size.y), colorLines);
    }

    for (int i = 0; i < 100; i += stepMini) {
        ImVec2 firstPos = ImFloor(pos + ImVec2(i * TIME_WIDTH * zoom * fontSize, 0));
        draw_list->AddLine(firstPos + ImVec2(0, headerHeight) * 0.7, firstPos + ImVec2(0, headerHeight), colorLines);
    }

    SetCursorScreenPos(canvas_pos + ImVec2(0, canvas_size.y - headerBottom));

    bool openAudioFile = false;
    bool openErrorBox = false;
    if (Button("Add", ImVec2(50, headerBottom))) {
        OpenPopup(POPUP_ADD_LAYER);
    }
    if (Button("Add", ImVec2(50, headerBottom))) {
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

    // todo: scrollbars
    // window->DrawList->AddRectFilled(grab_rect.Min, grab_rect.Max, grab_col, style.ScrollbarRounding);

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
