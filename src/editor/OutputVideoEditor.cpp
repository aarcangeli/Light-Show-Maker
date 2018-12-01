#define IMGUI_DEFINE_MATH_OPERATORS

#include "OutputVideoEditor.h"
#include "Application.h"
#include "AudioLoader.h"
#include "ImageLoader.h"
#include "IconsFontAwesome4.h"
#include "glad/glad.h"

using namespace sm;
using namespace sm::editor;
using namespace std;
using namespace ImGui;

void OutputVideoEditor::editorOf(shared_ptr<project::Project> proj) {
    drawContent(proj);
    topMenu(proj);
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

    // draw content
    drawCanvas(proj->canvas);
}

void OutputVideoEditor::topMenu(const shared_ptr<project::Project> &proj) const {
    ImVec2 oldPos = GetCursorScreenPos();
    float height = dpi * TOPBAR_HEIGHT;
    ImVec2 rect = ImVec2(ImFloor(GetContentRegionAvail().x), height);
    BeginChild("TopBar", rect, false, ImGuiWindowFlags_NoScrollbar);

    if (Button(ICON_FA_FILE_IMAGE_O, ImVec2(height, height))) openImage(proj);

    EndChild();
    SetCursorScreenPos(oldPos);
}

void OutputVideoEditor::openImage(const shared_ptr<project::Project> &proj) const {
    string outPath = gApp->getPath("png,jpg,jpeg");
    if (!outPath.empty()) {
        shared_ptr<media::Image> image = media::loadImage(outPath);
        if (image) {
            auto dec = make_shared<project::Decoration>();
            dec->width = image->width;
            dec->height = image->height;
            dec->posX = 0;
            dec->posY = 0;
            dec->type = project::IMAGE;

            dec->image = image;
            shared_ptr<project::LightGroup> selected = gApp->layerSelected();
            (selected ? selected->decorations : proj->canvas.decorations).push_back(dec);
        } else {
            gApp->error("Unable to open image '" + outPath + "'");
        }
    }
}

void OutputVideoEditor::drawCanvas(const project::Canvas &canvas) {
    for (auto &dec : canvas.decorations) {
        printDecoration(dec);
    }
}

void OutputVideoEditor::printDecoration(const std::shared_ptr<project::Decoration> &dec) {
    ImVec2 pos = canvasScreenPos + ImVec2(dec->posX, dec->posY);
    ImVec2 dim = ImVec2(dec->width, dec->height) * canvasSize / logicalSize;

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

        // coming
//        draw_list->PushTextureID(tex_id);
//        draw_list->PrimReserve(6, 4);
//        draw_list->PrimQuadUV(a, b, c, d, uv_a,uv_b, uv_c, uv_d, col);
//        draw_list->PopTextureID();
        drawList->AddImage(id, pos, pos + dim);
    }

    if (dec->type == project::LIGHT) {
        drawList->AddCircle(pos, dec->size, dec->color);
    }
}
