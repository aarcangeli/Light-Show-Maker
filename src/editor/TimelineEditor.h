#ifndef TIMELINEEDITOR_H
#define TIMELINEEDITOR_H

#define IMGUI_DEFINE_MATH_OPERATORS

#include "Canvas.h"
#include <imgui.h>
#include <AudioLoader.h>
#include <ScrollablePane.h>
#include <AudioDevice.h>

namespace sm {
class Application;
namespace editor {

class TimelineEditor {
    const char *POPUP_ADD_LAYER = "POPUP_ADD_LAYER";
    const char *MODAL_ADD_AUDIO = "Audio Track";
    const char *MODAL_ERROR = "Error!";

    const ImU32 COLOR_BG = IM_COL32(51, 51, 51, 255); // 333333
    const ImU32 COLOR_BG2 = IM_COL32(38, 38, 38, 255); // 262626
    const ImU32 COLOR_LAYER = IM_COL32(83, 83, 83, 255); // 535353
    const ImU32 COLOR_LAYER_SEL = IM_COL32(205, 209, 209, 255); // cdd1d1
    const ImU32 OVERLAY_WHITE = IM_COL32(255, 255, 255, 255 * 0.05);
    const ImU32 COLOR_LINE = IM_COL32(127, 127, 127, 255 * 0.4);

    const ImVec4 COLOR_TEXT = {1, 1, 1, 0.7f};

    const int POPUP_SETTINGS = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings;

    const int scrollBarHeight = 14;
    // multiplier of zoom and fontsize
    const float TIME_WIDTH = 10;

public:
    explicit TimelineEditor(Application *app);
    void reset();
    void editorOf(project::Canvas &canvas);

    float dpi;

private:
    media::AudioLoader loader;
    sm::Application *app;
    ScrollablePane scroll;

    std::string lastError;

    ImU32 setAlpha(ImU32 color, double alpha);
    void addAudioModal();

    void saveLastDirectory(const char *outPath) const;

    void errorBox();

    float headerTopHeight;
    float headerBotHeight;
    float leftSideWidth;
    float layerHeight;
    float lineDim;
    float delim1;

    // visible layers [firstIndex, indexMax)
    int firstIndex, indexMax;

    time_unit timeStep;

    // scale and offset
    ImVec2 offset, scale;

    void printContent(const project::Canvas &canvas, const ImRect &rect);
    void printLayerList(const project::Canvas &canvas, ImRect rect);
    void printLayer(project::LightGroup *group, ImRect rect);
    float getTimePos(time_unit time);
    void printTimeline(const project::Canvas &canvas, ImRect rect);
    std::string timeLabel(time_unit time) const;

    sm::media::AudioDevice device;
};

}
}

#endif //TIMELINEEDITOR_H
