#include "gui/WorldGeneratorUI.h"
#include "world/generationAlgorithms/PerlinNoise2D.h"
#include "world/generationAlgorithms/FlatFill.h"

WorldGeneratorUI::WorldGeneratorUI(WorldTerrainGenerator* generator) :
	worldGenerator(generator), 
    selectedLayerIndex(-1)
{
	
}

void WorldGeneratorUI::renderImGui()
{
    if (!worldGenerator) return;

    float screenWidth = ImGui::GetIO().DisplaySize.x;
    float screenHeight = ImGui::GetIO().DisplaySize.y;
    float panelWidth = screenWidth / 6.0f;

    ImGui::SetNextWindowPos(ImVec2(screenWidth - panelWidth, 0));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, screenHeight));

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("World Generator Settings", nullptr, windowFlags);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Generation Layers:");

    auto& layers = targetGenerator->generationLayers;

    if (ImGui::BeginListBox("##LayersList", ImVec2(-FLT_MIN, 200))) {
        for (int i = 0; i < layers.size(); i++) {
            // Dodajemy numer indeksu do etykiety, aby każda była unikalna dla ImGui
            std::string layerLabel = "[" + std::to_string(i) + "] " + layers[i]->layerName;

            bool isSelected = (i == selectedLayerIndex);
            if (ImGui::Selectable(layerLabel.c_str(), isSelected)) {
                selectedLayerIndex = i;
            }
        }
        ImGui::EndListBox();
    }

    // --- PRZYCISKI DODAWANIA ---
    if (ImGui::Button("Add Perlin")) {
        layers.push_back(std::make_unique<PerlinNoise2D>("Perlin", 0, 64, 12345));
        selectedLayerIndex = layers.size() - 1; // Automatycznie zaznacz nową warstwę
        std::cout << "[UI] Dodano warstwę Perlin. Łącznie: " << layers.size() << std::endl;
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Flat")) {
        layers.push_back(std::make_unique<FlatFill>("Flat", 0, 10));
        selectedLayerIndex = layers.size() - 1; // Automatycznie zaznacz nową warstwę
        std::cout << "[UI] Dodano warstwę Flat. Łącznie: " << layers.size() << std::endl;
    }

    ImGui::Separator();

    if (selectedLayerIndex >= 0 && selectedLayerIndex < layers.size()) {
        auto& selectedLayer = layers[selectedLayerIndex];

        ImGui::TextColored(ImVec4(0, 1, 1, 1), "Edit: %s", selectedLayer->layerName.c_str());

        ImGui::DragInt("Start Y", &selectedLayer->startY, 1, 0, 255);
        ImGui::DragInt("End Y", &selectedLayer->endY, 1, 0, 255);

        ImGui::Separator();

        selectedLayer->renderImGuiSettings();

        ImGui::Spacing();
        if (ImGui::Button("Remove Layer", ImVec2(-FLT_MIN, 0))) {
            layers.erase(layers.begin() + selectedLayerIndex);
            selectedLayerIndex = -1;
        }
    }
    else {
        ImGui::TextDisabled("No layer selected");
    }

    ImGui::SetCursorPosY(screenHeight - 40);
    if (ImGui::Button("REGENERATE WORLD", ImVec2(-FLT_MIN, 30))) {
        //W PRZYSZŁOŚCI DODAĆ REGENERACJE ŚWIATA
    }

    ImGui::End();
}