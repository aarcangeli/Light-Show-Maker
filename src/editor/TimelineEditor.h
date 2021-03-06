#ifndef TIMELINEEDITOR_H
#define TIMELINEEDITOR_H

#define IMGUI_DEFINE_MATH_OPERATORS

#include "core.h"
#include "Canvas.h"
#include "map"
#include "MultiKeypointDragger.h"
#include "KeypointDragger.h"
#include "functional"
#include <AudioLoader.h>
#include <ScrollablePane.h>
#include <AudioDevice.h>
#include <Serialization.h>

namespace sm {
namespace editor {

class TimelineEditor {
    using KeyChecker = std::function<bool(time_unit, std::shared_ptr<model::KeyPoint>)>;

    const char *POPUP_ADD_LAYER = "POPUP_ADD_LAYER";
    const char *MODAL_ADD_AUDIO = "Audio Track";

    const ImU32 COLOR_BG = IM_COL32(51, 51, 51, 255); // #333333
    const ImU32 COLOR_BG2 = IM_COL32(38, 38, 38, 255); // #262626
    const ImU32 COLOR_LAYER = IM_COL32(83, 83, 83, 255); // #535353
    const ImU32 COLOR_SEL_UNFOCUS = IM_COL32(0, 0, 0, 255 * 0.2);
    const ImU32 OVERLAY_WHITE = IM_COL32(255, 255, 255, 255 * 0.05);
    const ImU32 COLOR_LINE = IM_COL32(127, 127, 127, 255 * 0.4);
    const ImU32 COLOR_KEY = IM_COL32(127, 127, 127, 255 * 0.4);
    const ImU32 COLOR_KEY_SEL = IM_COL32(200, 200, 200, 255 * 0.8);
    const ImU32 COLOR_KEY_HOV = IM_COL32(127, 127, 127, 255 * 0.7);
    const ImU32 COLOR_KEY_OUTLINE = IM_COL32(50, 50, 50, 255 * 0.8);
    const ImU32 COLOR_KEY_RESIZE = IM_COL32(255, 0, 0, 255 * 0.8);
    const ImU32 COLOR_KEY_GRAPH = IM_COL32(255, 0, 0, 255 * 0.6);

    const ImU32 COLOR_CURSOR = IM_COL32(0, 255, 0, 255 * 0.8);
    const float SIZE_WAVE = 30;

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
    void editorOf(model::Canvas &canvas);

private:
    // for preview
    media::AudioLoader loader;
    ScrollablePane scroll{true, false};

    model::Canvas *canvas;

    ImU32 setAlpha(ImU32 color, double alpha);

    ImRect waveRect;
    ImRect layersRect;
    ImRect contentRect;

    float headerTopHeight;
    float headerBotHeight;
    float leftSideWidth;
    float layerHeight;
    float lineDim;
    float delim1;

    ImU32 btnActive, btnHover;

    bool isLayerListFocused;
    bool isContentFocused;

    // visible layers [firstIndex, indexMax)
    int firstIndex, indexMax;

    time_unit timeStep;

    KeypointDragger dragger;
    MultiKeypointDragger multiDragger;

    // scale and offset of timeline
    ImVec2 offset, scale;

    void printContent(model::Canvas &canvas, const ImRect &rect);
    void printLayerList(ImRect rect);

    void printLayer(int i, std::shared_ptr<model::Layer> group, ImRect rect);
    float getTimePosScreenPos(time_unit time);
    void printTimeline(const model::Canvas &canvas, ImRect rect);
    std::string timeLabel(time_unit time, bool withMills);

public:
    bool lookUpAtPos(ImVec2 pos, time_unit *time, int *layerIdx = nullptr);
    bool lookMousePos(ImVec2 pos, time_unit *time, int *layerIdx = nullptr);
private:

    // editor settings
    bool snapCursor = true;
    time_unit snapTime = TIME_UNITS;

    bool isDraggingPosition = false;
    bool withDetails;

    // box selection
    bool boxSelecting = false;
    bool boxSubtract = false;
    ImVec2 boxStart;
    std::vector<std::shared_ptr<model::KeyPoint>> boxOldSelection;
    time_unit timeStart;
    int layerStart;

    void drawKey(std::shared_ptr<model::Layer> group,
                 std::shared_ptr<model::KeyPoint> &key,
                 const ImRect &rect,
                 bool isHover);

    float getTimeScaleX() const;

public:
    SERIALIZATION_START {
        ser.serialize("offset.x", offset.x);
        ser.serialize("offset.y", offset.y);
        ser.serialize("scale.x", scale.x);
        ser.serialize("scale.y", scale.y);
        if (ser.DESERIALIZING) {
            scroll.setOffset(offset);
            scroll.setScale(scale);
        }
    };

    bool findPlacableKeyPos(ImVec2 &pos, time_unit &start, time_unit duration, int32_t &layerIdx);

    model::Canvas *getCanvas() {
        assert(canvas);
        return canvas;
    }

    bool isInsideContent(const ImVec2 &pos) const;
    void printMediaControls(ImRect rect);
    float getTimeOffsetX() const;

    time_unit moveSnapped(time_unit input, KeyChecker checker);
    std::vector<std::shared_ptr<model::Layer>> layerToDelete;

private:
    bool findBestSnap(time_unit input, time_unit &best, time_unit &bestDest, const KeyChecker &checker) const;

    void snapItem(time_unit dest, time_unit input, const std::shared_ptr<model::KeyPoint> &key,
                  time_unit &best, time_unit &bestDest, KeyChecker checker) const;

    void printWave(ImRect rect);

    void deleteKeypoints();
    void orderLayers();
};

}
}

#endif //TIMELINEEDITOR_H
