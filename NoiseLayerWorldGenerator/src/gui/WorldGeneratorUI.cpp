#include "gui/WorldGeneratorUI.h"
#include "world/generationAlgorithms/PerlinNoise2D.h"
#include "world/generationAlgorithms/FlatFill.h"
#include <world/WorldConfig.h>
#include <world/World.h>

WorldGeneratorUI::WorldGeneratorUI(WorldTerrainGenerator* generator, World* world) :
	worldGenerator(generator), 
    world(world),
    selectedLayerIndex(-1)
{
	
}

void WorldGeneratorUI::renderImGui()
{
    if (!worldGenerator) return;

	bool isLoading = (world->getCurrentState() == WorldState::LOADING);

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

    auto& layers = worldGenerator->generationLayers;

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

    if (ImGui::Button("Add Perlin")) {
        layers.push_back(std::make_unique<PerlinNoise2D>("Perlin", 0, 64, 12345));
        std::cout << "+[UI] Dodano warstwe Perlin. Lacznie: " << layers.size() << std::endl;
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Flat")) {
        layers.push_back(std::make_unique<FlatFill>("Flat", 0, 10));
        std::cout << "+[UI] Dodano warstwe Flat. Lacznie: " << layers.size() << std::endl;
    }

    ImGui::Separator();

    if (selectedLayerIndex >= 0 && selectedLayerIndex < layers.size()) {
        auto& selectedLayer = layers[selectedLayerIndex];

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


    if (isLoading) { ImGui::BeginDisabled(); }

    if (ImGui::Button("REGENERATE WORLD", ImVec2(-FLT_MIN, 30))) {
		world->regenerateWorld();
    }

    if (isLoading) { ImGui::EndDisabled(); }


    ImGui::End();

    if (isLoading) {
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

        ImGuiWindowFlags overlayFlags = ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoMove;

        ImGui::SetNextWindowBgAlpha(0.85f);

        if (ImGui::Begin("LoadingOverlay", nullptr, overlayFlags)) {
            ImGui::Text("Generating World... Please Wait");
            ImGui::Spacing();

            float progress = world->getGenerationProgress();
            ImGui::ProgressBar(progress, ImVec2(350.0f, 25.0f));
        }
        ImGui::End();
    }
}