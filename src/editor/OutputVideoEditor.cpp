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

void OutputVideoEditor::editorOf(shared_ptr<project::Project> proj) {
    mouseClicked = IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && IsMouseClicked(0);
    windowFocused = IsWindowFocused(ImGuiHoveredFlags_ChildWindows);
    mousePos = GetMousePos();
    decorationHover.reset();
    topMenu(proj);
    drawContent(proj);
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

void OutputVideoEditor::drawContent(std::shared_ptr<project::Project> proj) {
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

void OutputVideoEditor::topMenu(const shared_ptr<project::Project> &proj) {
    ImVec2 oldPos = GetCursorScreenPos();
    float height = dpi * TOPBAR_HEIGHT;
    ImVec2 rect = ImVec2(ImFloor(GetContentRegionAvail().x), height);
    BeginChild("TopBar", rect, false, ImGuiWindowFlags_NoScrollbar);

    if (Button(ICON_FA_FILE_IMAGE_O, ImVec2(height, height))) openImage(proj);
    SameLine();
    if (Button(ICON_FA_LIGHTBULB_O, ImVec2(height, height))) {
        auto dec = make_shared<project::Decoration>();
        dec->type = project::LIGHT;
        dec->posX = logicalSize.x / 2;
        dec->posY = logicalSize.y / 2;
        dec->size = 30;
        dec->color = 0xffffffff; // white
        append(proj, dec);
        dragger.startEditing(dec);
        gApp->getSelection().decorations.set(dec);
    }

    EndChild();
    SetCursorScreenPos(oldPos);
}

void OutputVideoEditor::openImage(const shared_ptr<project::Project> &proj) const {
    string outPath = gApp->getPath("png,jpg,jpeg", false);
    if (!outPath.empty()) {
        shared_ptr<media::Image> image = media::loadImage(outPath);
        if (image) {
            auto dec = make_shared<project::Decoration>();
            dec->type = project::IMAGE;
            dec->width = image->width;
            dec->height = image->height;
            dec->posX = 0;
            dec->posY = 0;
            dec->image = image;
            append(proj, dec);
        } else {
            gApp->error("Unable to open image '" + outPath + "'");
        }
    }
}

void
OutputVideoEditor::append(const shared_ptr<project::Project> &proj, const shared_ptr<project::Decoration> &dec) const {
    auto &selected = gApp->getSelection().layers;
    if (selected.size()) {
        selected[0]->decorations.push_back(dec);
    } else {
        proj->canvas.decorations.push_back(dec);
    }
}

void OutputVideoEditor::drawCanvas(project::Canvas &canvas) {
    drawVector(1, canvas.decorations);
    for (auto &group : canvas.groups) {
        time_unit position = gApp->getPlayer().playerPosition();
        float alpha = position ? group->computeEasing(position) : 1;
        drawVector(alpha, group->decorations);
    }
}

void OutputVideoEditor::drawVector(float alpha, vector<shared_ptr<project::Decoration>> &array) {
    ImGuiIO &io = GetIO();
    auto &selected = gApp->getSelection().decorations;
    auto it = array.begin();
    while (it != array.end()) {
        shared_ptr<project::Decoration> el = *it;
        if (el == decorationToDelete) {
            it = array.erase(it);
            continue;
        }
        if (IsKeyPressed(GLFW_KEY_PAGE_UP) && it != array.end() - 1) {
            if (!selected.empty() && selected[0] == el) {
                it = array.erase(it);
                it = array.insert(it + 1, el);
                it -= 2;
            }
        }
        printDecoration(alpha, el);
        if (IsKeyPressed(GLFW_KEY_PAGE_DOWN) && it != array.begin()) {
            if (!selected.empty() && selected[0] == el) {
                it = array.erase(it);
                it = array.insert(it - 1, el);
                it++;
            }
        }
        if (io.KeyCtrl && IsKeyPressed(GLFW_KEY_D)) {
            if (!selected.empty() && selected[0] == el) {
                auto copy = std::make_shared<project::Decoration>(*el);
                it = array.insert(it + 1, copy);
                it--;
            }
        }
        if (mouseClicked && io.KeyAlt) {
            if (!selected.empty() && selected[0] == el) {
                auto copy = std::make_shared<project::Decoration>(*el);
                it = array.insert(it + 1, copy);
                selected.set(copy);
            }
        }
        it++;
    }
}

void OutputVideoEditor::printDecoration(float decAlpha, const std::shared_ptr<project::Decoration> &dec) {
    ImVec2 pos = canvasScreenPos + ImVec2(dec->posX, dec->posY) * getLogicalScale();
    ImRect region = getDecorationRegion(dec);

    ImDrawList *drawList = GetWindowDrawList();
    if (dec->type == project::IMAGE) {
        ImTextureID &id = dec->textureId;

        // todo: better texture managment
        if (!id) {
            GLuint texture = 0;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            shared_ptr<media::Image> &im = dec->image;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, im->width, im->height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         im->pixels.data());
            id = (ImTextureID) (size_t) texture;
        }

        ImVec2 dim = ImVec2(dec->width, dec->height) * getLogicalScale();
        ImVec2 max = pos + dim;
        // coming
//        draw_list->PushTextureID(tex_id);
//        draw_list->PrimReserve(6, 4);
//        draw_list->PrimQuadUV(a, b, c, d, uv_a,uv_b, uv_c, uv_d, col);
//        draw_list->PopTextureID();
        drawList->AddImage(id, pos, pos + dim);
    }

    if (dec->type == project::LIGHT) {
        uint32_t color = dec->color;
        uint32_t alpha = color >> 24;
        color &= 0xffffff;
        alpha = uint32_t(alpha * decAlpha);
        color |= alpha << 24;
        drawList->AddCircleFilled(pos, dec->size / logicalSize.y * canvasSize.y, color);
    }

    if (region.Contains(mousePos)) {
        decorationHover = dec;
    }

    if (lastDecorationHover == dec) {
        dragger.drawPlaceBox(region, true);
    }
}

ImVec2 OutputVideoEditor::getLogicalScale() const { return canvasSize / logicalSize; }

ImRect OutputVideoEditor::getDecorationRegion(const std::shared_ptr<project::Decoration> &dec) {
    ImVec2 pos = canvasScreenPos + ImVec2(dec->posX, dec->posY) * getLogicalScale();
    ImVec2 dim;
    if (dec->type == project::IMAGE) {
        dim = ImVec2(dec->width, dec->height) * getLogicalScale();
    }
    if (dec->type == project::LIGHT) {
        float size = dec->size / logicalSize.y * canvasSize.y;
        dim = ImVec2(size, size) * 2;
        pos -= dim / 2;
    }
    return {pos, pos + dim};
}

void OutputVideoEditor::deleteDecoration(const shared_ptr<project::Decoration> &dec) {
    decorationToDelete = dec;
}
