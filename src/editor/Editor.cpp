#include "Editor.h"
#include "math.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <stdexcept>

using namespace sm;
using namespace sm::editor;
using namespace sm::project;
using namespace ImGui;

Editor::Editor() = default;

void Editor::editorOf(Project *project) {
    return;
    static int test = 0;

//    {
//        BeginGroup();
//        if (TreeNode("tree")) {
//            ImGui::Text("space");
//            ImGui::Text("space");
//            ImGui::Text("space");
//            ImGui::Text("space");
//            TreePop();
//        }
//        EndGroup();
//    }
//
//    SameLine();
//
//    {
//        BeginGroup();
//        if (TreeNode("tree2")) {
//            ImGui::Text("space");
//            ImGui::Text("space");
//            ImGui::Text("space");
//            ImGui::Text("space");
//            TreePop();
//        }
//        EndGroup();
//    }


    ImGui::Columns(2);
    if (firstShow) SetColumnWidth(0, 300);
    ImGui::Separator();

    ImGui::Text("Header1");
    ImGui::NextColumn();
    ImGui::Text("Header2");
    ImGui::NextColumn();
    ImGui::Separator();

    editorComponent(project->canvas);
    if (TreeNode("test-3")) {
        InputInt("test", &test);
        TreePop();
    }

    if (TreeNode("test-343")) {
        InputInt("testtest2", &test, 0, 0);
        InputInt("testtest3", &test, 0, 0);
        InputInt("testtest4", &test, 0, 0);
        InputInt("testtest5", &test, 0, 0);
        InputInt("testtest6", &test, 0, 0);
        InputInt("testtest76", &test, 0, 0);
        TreePop();
    }

    ImGui::NextColumn();
    ImGui::Text("space");
    ImGui::Text("space");
    ImGui::Text("space");
    ImGui::Text("space");

    ImGui::NextColumn();
    Separator();
    ImGui::Text("spaceL");
    ImGui::Text("spaceL");
    ImGui::Text("spaceL");
    ImGui::Text("spaceL");

    ImGui::NextColumn();
    ImGui::Text("space");
    ImGui::Text("space");
    ImGui::Text("space");
    ImGui::Text("space");

    Columns(1);
    Separator();
    firstShow = false;
}

void Editor::editorComponent(Canvas &canvas) {
    editorComponent("duration", canvas.duration);
}

void Editor::editorComponent(const char *label, time_unit &time) {
    float button_size = GetFrameHeight();
    ImGuiStyle &style = GetStyle();

    BeginGroup();
    PushID(label);
    PushItemWidth(ImMax(1.0f, CalcItemWidth() - (button_size + style.ItemInnerSpacing.x) * 2));

    time_unwrapped u = time_unwrap(time);

    char buf[64];
    ImFormatString(buf, IM_ARRAYSIZE(buf), "%llu:%02llu:%02llu:%03llu", u.hours, u.minutes, u.seconds, u.mills);
    if (InputText("", buf, IM_ARRAYSIZE(buf), 0)) {
        parseTime(buf, time);
    }

    SameLine(0, style.ItemInnerSpacing.x);
    TextUnformatted(label, FindRenderedTextEnd(label));

    PopItemWidth();
    PopID();
    EndGroup();

}

void Editor::parseTime(const char *_buf, sm::time_unit &time) {
    std::string buf = _buf;

    skipWhitespace(buf);
    if (!buf[0]) return;

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
                return;
            }
        }
        scanned++;
        if (!buf[0]) break;
        if (buf[0] != ':') return;
        buf = buf.substr(1);
        skipWhitespace(buf);
    }

    // ensure eof
    if (scanned == 0 || scanned > 4 || buf[0]) return;

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
        float itm = p[scanned - 4];
        u.hours += static_cast<uint64_t>(itm);
        u.minutes += (uint32_t) (itm * 1000) % 1000;
    }

    time_unit res = time_wrap(u);
    if (res < max_time) {
        time = res;
    }
}

inline void Editor::skipWhitespace(std::string &buf) {
    size_t pos = 0;
    while (ImCharIsBlankA(buf[pos]))
        pos++;
    if (pos) buf = buf.substr(pos);
}
