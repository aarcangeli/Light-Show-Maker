#include <memory>

#define IMGUI_DEFINE_MATH_OPERATORS

#include "use_glad.h"
#include "OutputVideoEditor.h"
#include "Application.h"
#include "AudioLoader.h"
#include "ImageLoader.h"
#include "IconsFontAwesome4.h"
#include "imgui_internal.h"

using namespace sm;
using namespace sm::editor;
using namespace std;
using namespace ImGui;

OutputVideoEditor::OutputVideoEditor() : dragger(this), scrollablePane(true, true) {
}

void OutputVideoEditor::init(const std::shared_ptr<model::Project> &_proj) {
    proj = _proj;
}

void OutputVideoEditor::showEditor() {
    mouseClicked = IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && IsMouseClicked(0);
    windowFocused = IsWindowFocused(ImGuiHoveredFlags_ChildWindows);
    mousePos = GetMousePos();
    decorationHover.reset();
    topMenu();
    drawContent();
    if (mouseClicked) {
        dragger.startEditing(decorationHover);
        if (decorationHover) {
            gApp->getSelection().decorations.set(decorationHover);
        } else {
            gApp->getSelection().decorations.reset();
        }
    }
    lastDecorationHover = decorationHover;
    dragger.update();
}

void OutputVideoEditor::drawContent() {
    model::Settings &settings = proj->settings;
    const float RATIO = (float) settings.width / settings.height;

    // compute size and position
    float padding = CONTENT_PADDING * dpi;
    const ImVec2 &avail = ImFloor(GetContentRegionAvail()) - ImVec2(padding * 2, padding * 2);
    canvasSize = avail;
    canvasScreenPos = ImFloor(GetCursorScreenPos() + ImVec2(padding, padding));
    if (canvasSize.x > canvasSize.y * RATIO) {
        canvasSize.x = canvasSize.y * RATIO;
        canvasScreenPos.x += (avail.x - canvasSize.x) / 2;
    }
    if (canvasSize.y > canvasSize.x / RATIO) {
        canvasSize.y = canvasSize.x / RATIO;
        canvasScreenPos.y += (avail.y - canvasSize.y) / 2;
    }

    GetWindowDrawList()->AddRect(canvasScreenPos, canvasScreenPos + canvasSize, COLOR_BORDER);

    drawCanvas(proj->canvas);
}

void OutputVideoEditor::topMenu() {
    model::Settings &settings = proj->settings;
    ImVec2 oldPos = GetCursorScreenPos();
    float height = dpi * TOPBAR_HEIGHT;
    ImVec2 rect = ImVec2(ImFloor(GetContentRegionAvail().x), height);
    BeginChild("TopBar", rect, false, ImGuiWindowFlags_NoScrollbar);

    if (Button(ICON_FA_FILE_IMAGE_O, ImVec2(height, height))) openImage(proj);
    SameLine();
    if (Button(ICON_FA_LIGHTBULB_O, ImVec2(height, height))) {
        auto dec = make_shared<model::Decoration>();
        dec->type = model::LIGHT;
        dec->posX = settings.width / 2.f;
        dec->posY = settings.height / 2.f;
        dec->size = 30;
        dec->color = 0xffffffff; // white
        append(proj, dec);
        dragger.startEditing(dec);
        gApp->getSelection().decorations.set(dec);
    }

    EndChild();
    SetCursorScreenPos(oldPos);
}

void OutputVideoEditor::openImage(const shared_ptr<model::Project> &proj) const {
    string outPath = gApp->getPath("png,jpg,jpeg", false);
    if (!outPath.empty()) {
        shared_ptr<media::Image> image = media::loadImage(outPath);
        if (image) {
            auto dec = make_shared<model::Decoration>();
            dec->type = model::IMAGE;
            dec->width = image->width;
            dec->height = image->height;
            dec->posX = 0;
            dec->posY = 0;
            dec->image = outPath;
            append(proj, dec);
        } else {
            gApp->error("Unable to open image '" + outPath + "'");
        }
    }
}

void
OutputVideoEditor::append(const shared_ptr<model::Project> &proj, const shared_ptr<model::Decoration> &dec) const {
    auto &selected = gApp->getSelection().layers;
    if (!selected.empty()) {
        selected[0]->decorations.push_back(dec);
    } else {
        proj->canvas.decorations.push_back(dec);
    }
}

void OutputVideoEditor::drawCanvas(model::Canvas &canvas) {
    bool withLayerRestriction = true;
    drawVector(1, canvas.decorations, !withLayerRestriction);
    for (auto &group : canvas.groups) {
        time_unit position = gApp->getPlayer().playerPosition();
        float alpha = position ? group->computeEasing(position) : 1;
        drawVector(alpha, group->decorations, !withLayerRestriction || group->isSelected);
    }
}

void OutputVideoEditor::drawVector(float alpha, std::vector<std::shared_ptr<model::Decoration>> &array, bool isSelected) {
    ImGuiIO &io = GetIO();
    auto &selected = gApp->getSelection().decorations;
    auto it = array.begin();
    while (it != array.end()) {
        shared_ptr<model::Decoration> el = *it;
        if (el == decorationToDelete) {
            it = array.erase(it);
            continue;
        }
        bool isDecorationSelected = !selected.empty() && selected[0] == el;
        if (IsKeyPressed(GLFW_KEY_PAGE_UP) && it != array.end() - 1) {
            if (isDecorationSelected) {
                it = array.erase(it);
                it = array.insert(it + 1, el);
                it -= 2;
            }
        }
        printDecoration(alpha, el, isSelected);
        if (IsKeyPressed(GLFW_KEY_PAGE_DOWN) && it != array.begin()) {
            if (isDecorationSelected) {
                it = array.erase(it);
                it = array.insert(it - 1, el);
                it++;
            }
        }
        if (io.KeyCtrl && IsKeyPressed(GLFW_KEY_D)) {
            if (isDecorationSelected) {
                auto copy = std::make_shared<model::Decoration>(*el);
                it = array.insert(it + 1, copy);
                it--;
            }
        }
        if (mouseClicked && io.KeyAlt) {
            if (isDecorationSelected) {
                auto copy = std::make_shared<model::Decoration>(*el);
                it = array.insert(it + 1, copy);
                selected.set(copy);
            }
        }
        it++;
    }
}

void OutputVideoEditor::printDecoration(float decAlpha, const std::shared_ptr<model::Decoration> &dec, bool isSelected) {
    PROFILE_BLOCK("OutputVideoEditor::printDecoration", 0.05);
    ImVec2 pos = canvasScreenPos + (OFFSET + ImVec2(dec->posX, dec->posY)) * getLogicalScale();
    ImRect region = getDecorationRegion(dec);

    ImDrawList *drawList = GetWindowDrawList();
    if (dec->type == model::IMAGE) {
        ImTextureID id = (ImTextureID) (size_t) gApp->getResourceManager().loadTexture(dec->image);

        ImVec2 dim = ImVec2(dec->width, dec->height) * getLogicalScale();
        ImVec2 max = pos + dim;
        // coming
//        draw_list->PushTextureID(tex_id);
//        draw_list->PrimReserve(6, 4);
//        draw_list->PrimQuadUV(a, b, c, d, uv_a,uv_b, uv_c, uv_d, col);
//        draw_list->PopTextureID();
        drawList->AddImage(id, pos, pos + dim);
    }

    if (dec->type == model::LIGHT) {
        uint32_t color = dec->color;
        uint32_t alpha = color >> 24;
        color &= 0xffffff;
        alpha = uint32_t(alpha * decAlpha);
        color |= alpha << 24;
        drawList->AddCircleFilled(pos, dec->size / proj->settings.height * canvasSize.y, color);
    }

    if (isSelected && region.Contains(mousePos)) {
        decorationHover = dec;
    }

    if (lastDecorationHover == dec) {
        dragger.drawPlaceBox(region, true);
    }
}

ImVec2 OutputVideoEditor::getLogicalScale() const {
    return canvasSize / ImVec2(proj->settings.width, proj->settings.height);
}

ImRect OutputVideoEditor::getDecorationRegion(const std::shared_ptr<model::Decoration> &dec) {
    model::Settings &settings = proj->settings;
    ImVec2 pos = canvasScreenPos + (OFFSET + ImVec2(dec->posX, dec->posY)) * getLogicalScale();
    ImVec2 dim;
    if (dec->type == model::IMAGE) {
        dim = ImVec2(dec->width, dec->height) * getLogicalScale();
    }
    if (dec->type == model::LIGHT) {
        float size = dec->size / settings.height * canvasSize.y;
        dim = ImVec2(size, size) * 2;
        pos -= dim / 2;
    }
    return {pos, pos + dim};
}

void OutputVideoEditor::deleteDecoration(const shared_ptr<model::Decoration> &dec) {
    decorationToDelete = dec;
}
