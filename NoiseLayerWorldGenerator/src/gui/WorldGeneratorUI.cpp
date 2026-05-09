#include "gui/WorldGeneratorUI.h"

WorldGeneratorUI::WorldGeneratorUI()
{
	//DODAĆ POTEM JAKIS POCZĄTKOWY LAYER
}

void WorldGeneratorUI::renderImGui()
{
	float screenWidth = ImGui::GetIO().DisplaySize.x;
	float screenHeight = ImGui::GetIO().DisplaySize.y;

	float panelWidth = screenWidth / 6.0f;
	float panelHeight = screenHeight;

	ImGui::SetNextWindowPos(ImVec2(screenWidth - panelWidth, 0));
	ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoSavedSettings;

	ImGui::Begin("World Generator Settings", nullptr, windowFlags);

	ImGui::Text("Generation Layers:");
	if (ImGui::BeginListBox("##LayersList", ImVec2(-FLT_MIN, 200))) {
		for (int i = 0; i < generationLayers.size(); i++) {
			std::string layerLabel = generationLayers[i]->layerName + " (Y: " + std::to_string(generationLayers[i]->startY) + " - " + std::to_string(generationLayers[i]->endY) + " )";

			bool isSelected = (i == selectedLayerIndex);

			if (ImGui::Selectable(layerLabel.c_str(), isSelected)) {
				selectedLayerIndex = i;
			}
		}
		ImGui::EndListBox();
	}

	ImGui::Separator();

	if(selectedLayerIndex >= 0 && selectedLayerIndex < generationLayers.size()){
		ImGui::Text("Selected Layer Settings: %s", generationLayers[selectedLayerIndex]->layerName.c_str());

		ImGui::DragInt("Start Y", &generationLayers[selectedLayerIndex]->startY, 1, 0, 255);
		ImGui::DragInt("End Y", &generationLayers[selectedLayerIndex]->endY, 1, 0, 255);

		ImGui::Separator();

		generationLayers[selectedLayerIndex]->renderImGuiSettings();
	}
	else {
		ImGui::Text("No layer selected");
	}

	ImGui::End();
}

void WorldGeneratorUI::generateColumn()
{
	for (auto& layer : generationLayers) {
		layer->applyToColumn();
	}
}