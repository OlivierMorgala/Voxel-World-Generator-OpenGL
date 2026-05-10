#include "world/generationAlgorithms/FlatFill.h"

FlatFill::FlatFill(std::string name, int startY, int endY)
    : GenerationAlgorithm(name, startY, endY)
{

}

void FlatFill::applyToColumn(ChunkColumn& column) {
    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            for (int y = startY; y <= endY; y++) {
                column.setBlock(x, y, z, 1);
            }
        }
    }
}

void FlatFill::renderImGuiSettings() {
    if (ImGui::TreeNode(layerName.c_str())) {
        ImGui::DragInt("Start Y", &startY, 1, 0, 255);
        ImGui::DragInt("End Y", &endY, 1, 0, 255);
        ImGui::TreePop();
    }
}

