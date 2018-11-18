#include "Editor.h"

#include <imgui.h>

using namespace sm::editor;
using namespace sm::project;

Editor::Editor() = default;

void Editor::editorOf(Project *project) {
    ImGui::Begin(project->getName().c_str(), nullptr, ImGuiWindowFlags_None);
    if (ImGui::TreeNode("Basic")) {
        //ImGui::Button("Edit Timeline");
        ImGui::TreePop();
    }
    ImGui::End();
}
