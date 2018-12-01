#include "OutputVideoEditor.h"
#include "imgui.h"
#include "Application.h"
#include "AudioLoader.h"
#include "ImageLoader.h"

using namespace sm;
using namespace sm::editor;
using namespace std;
using namespace ImGui;

void OutputVideoEditor::editorOf(shared_ptr<project::Project> proj) {
    if (Button("Open")) {
        string outPath = gApp->getPath("png,jpg,jpeg");
        if (!outPath.empty()) {
            shared_ptr<media::Image> image = media::loadImage(outPath);
            if (image) {
                auto dec = std::make_shared<project::Decoration>();
                dec->image = image;
                shared_ptr<project::LightGroup> selected = gApp->layerSelected();
                (selected ? selected->decorations : proj->canvas.decorations).push_back(dec);
            }
        }
    }
}
