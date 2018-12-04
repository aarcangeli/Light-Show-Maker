#ifndef TIMELINEEDITOR_H
#define TIMELINEEDITOR_H

#define IMGUI_DEFINE_MATH_OPERATORS

#include "core.h"
#include "Canvas.h"
#include "map"
#include "KeypointDragger.h"
#include <AudioLoader.h>
#include <ScrollablePane.h>
#include <AudioDevice.h>

namespace sm {
namespace editor {

class TimelineEditor {

    const char *POPUP_ADD_LAYER = "POPUP_ADD_LAYER";
    const char *MODAL_ADD_AUDIO = "Audio Track";

    const ImU32 COLOR_BG = IM_COL32(51, 51, 51, 255); // #333333
    const ImU32 COLOR_BG2 = IM_COL32(38, 38, 38, 255); // #262626
    const ImU32 COLOR_LAYER = IM_COL32(83, 83, 83, 255); // #535353
    const ImU32 COLOR_SEL_UNFOCUS = IM_COL32(0, 0, 0, 255 * 0.2);
    const ImU32 OVERLAY_WHITE = IM_COL32(255, 255, 255, 255 * 0.05);
    const ImU32 COLOR_LINE = IM_COL32(127, 127, 127, 255 * 0.4);
    const ImU32 COLOR_KEY = IM_COL32(127, 127, 127, 255 * 0.4);
    const ImU32 COLOR_KEY_HOV = IM_COL32(127, 127, 127, 255 * 0.7);
    const ImU32 COLOR_KEY_OUTLINE = IM_COL32(50, 50, 50, 255 * 0.8);
    const ImU32 COLOR_KEY_RESIZE = IM_COL32(255, 0, 0, 255 * 0.8);
    const ImU32 COLOR_KEY_GRAPH = IM_COL32(255, 0, 0, 255 * 0.6);

    const ImU32 COLOR_CURSOR = IM_COL32(0, 255, 0, 255 * 0.8);

    const float COLOR_KEY_RADIUS = 2;
    const float COLOR_RESIZE_HANDLE_DIM = 4;
    const float CURVE_RESOLUTION = 2;

    const ImVec4 COLOR_TEXT = {1, 1, 1, 0.7f};

    const int POPUP_SETTINGS = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings;

    const int scrollBarHeight = 14;
    // multiplier of zoom and fontsize
    const float TIME_WIDTH = 10;

public:
    explicit TimelineEditor();
    void reset();
    void editorOf(project::Canvas &canvas);

    float dpi;

private:
    media::AudioLoader loader;
    ScrollablePane scroll;

    project::Canvas *canvas;

    ImU32 setAlpha(ImU32 color, double alpha);
    void addAudioModal();

    ImRect contentRect;

    float headerTopHeight;
    float headerBotHeight;
    float leftSideWidth;
    float layerHeight;
    float lineDim;
    float delim1;

    ImU32 btnActive, btnHover;

    bool isLayerListFocused;

    // visible layers [firstIndex, indexMax)
    int firstIndex, indexMax;

    time_unit timeStep;

    KeypointDragger dragger;

    // scale and offset of timeline
    ImVec2 offset, scale;

    void printContent(project::Canvas &canvas, const ImRect &rect);
    void printLayerList(const project::Canvas &canvas, ImRect rect);

    void printLayer(std::shared_ptr<project::LightGroup> group, ImRect rect);
    float getTimePosScreenPos(time_unit time);
    void printTimeline(const project::Canvas &canvas, ImRect rect);
    std::string timeLabel(time_unit time, bool withMills);

    sm::media::AudioDevice device;
    std::shared_ptr<project::LightGroup> selection;

    void deleteTrack(const std::shared_ptr<project::LightGroup> &group);

    bool lookUpAtPos(ImVec2 pos, time_unit *time, int *layerIdx = nullptr);
    bool lookMousePos(ImVec2 pos, time_unit *time, int *layerIdx = nullptr);

    // editor settings
    bool snapCursor = true;
    time_unit snapTime = TIME_UNITS;

    bool isDraggingPosition = false;

    void drawKey(std::shared_ptr<project::LightGroup> group,
                 std::shared_ptr<project::KeyPoint> &key,
                 const ImRect &rect,
                 bool isHover);

    float getTimeScaleX() const;

public:
    bool findPlacableKeyPos(ImVec2 &pos, time_unit &start, time_unit duration, int32_t &layerIdx);

    project::Canvas *getCanvas() {
        assert(canvas);
        return canvas;
    }

    bool isInsideContent(const ImVec2 &pos) const;
    void printMediaControls(ImRect rect);
    float getTimeOffsetX() const;
};

}
}

#endif //TIMELINEEDITOR_H
