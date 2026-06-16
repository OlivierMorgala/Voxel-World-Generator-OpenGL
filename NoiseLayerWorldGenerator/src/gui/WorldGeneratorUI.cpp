#include "gui/WorldGeneratorUI.h"
#include "world/generationAlgorithms/TerrainModifiers.h"
#include "world/generationAlgorithms/PerlinNoise2D.h"
#include "world/generationAlgorithms/PerlinNoise3D.h"
#include "world/generationAlgorithms/FlatFill.h"
#include "world/generationAlgorithms/SimplexNoise.h"
#include <world/WorldConfig.h>
#include <world/World.h>
#include <managers/SceneManager.h>
#include <scenes/LoadingScene.h>
#include <random>

WorldGeneratorUI::WorldGeneratorUI(WorldTerrainGenerator* generator, World* world) :
    worldGenerator(generator),
    world(world),
    selectedLayerIndex(-1)
{

}

void WorldGeneratorUI::renderImGui(bool isMenuOpen)
{
    if (!worldGenerator) return;


    float deltaTime = ImGui::GetIO().DeltaTime;
    float animationSpeed = 3.5f;

    if (isMenuOpen) {
        openAnimationProgress += animationSpeed * deltaTime;
        if (openAnimationProgress > 1.0f) { openAnimationProgress = 1.0f;  }
    }
    else 
    {
        openAnimationProgress -= animationSpeed * deltaTime;
        if (openAnimationProgress < 0.0f) { openAnimationProgress = 0.0f; }
    }

    if (openAnimationProgress <= 0.0f) {
        return;
    }

    float easedProgress = 1.0f - std::pow(1.0f - openAnimationProgress, 3.0f);

    // Obliczanie wymiarów i pozycji panelu
	bool isLoading = (world->getCurrentState() == WorldState::LOADING);

    float screenWidth = ImGui::GetIO().DisplaySize.x;
    float screenHeight = ImGui::GetIO().DisplaySize.y;
    float panelWidth = screenWidth / 4.0f;

    float currentX = screenWidth - (panelWidth * easedProgress);

    ImGui::SetNextWindowPos(ImVec2(currentX, 0));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, screenHeight));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.05f, 0.85f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("World Generator Settings", nullptr, windowFlags);

    ImGui::SameLine(ImGui::GetWindowWidth() - 65.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));

    if (ImGui::Button("EXIT", ImVec2(55.0f, 0.0f))) {
        SceneManager::getInstance().popScene();
    }

    ImGui::PopStyleColor(3);

    ImGui::Separator();

    // Rysowanie jasnej linii po lewej stronie
    ImVec2 p_min = ImGui::GetWindowPos();
    ImVec2 p_max = ImVec2(p_min.x + 3.0f, p_min.y + screenHeight);
    ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, IM_COL32(100, 255, 100, 255));

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);

    ImGui::TextColored(ImVec4(1, 1, 1, 1), "Generator Settings");
    ImGui::Separator();

    ImGui::PushItemWidth(230.0f);
    ImGui::InputInt("Global Seed", &config.worldSeed);
    ImGui::PopItemWidth();

    ImGui::SameLine();

    if (ImGui::Button("Randomize", ImVec2(110.0f, 0.0f))) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(-2147483640, 2147483640);
        config.worldSeed = distr(gen);
    }

    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::SliderInt("Render Distance", &config.renderDistance, 1, 60);

    ImGui::Spacing();
    ImGui::Spacing();

    std::vector<TerrainLayer>& layers = worldGenerator->generationLayers;

    // Flaga do automatycznego focusowania zakładki ze szczegółami
    static bool activateDetailsTab = false;

    // Początek paska zakładek
    if (ImGui::BeginTabBar("GeneratorTabs")) {

        // ZAKŁADKA 1: WARSTWY
        if (ImGui::BeginTabItem("Layers")) {

            if (ImGui::Button("+", ImVec2(30, 0))) {
                ImGui::OpenPopup("Add Layer");
            }
            ImGui::SameLine();
            if (ImGui::Button("-", ImVec2(30, 0)) && selectedLayerIndex >= 0 && selectedLayerIndex < layers.size()) {
                layers.erase(layers.begin() + selectedLayerIndex);
                selectedLayerIndex = -1;
            }

            if (ImGui::BeginPopupModal("Add Layer", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                static char nameBuf[64] = "New Layer";
                static int layerType = 0;
                static int start_y = 0, end_y = 64;

                static int currentBlockIndex = 1;
				static bool isCreatingNewBlock = false;

				static char newBlockName[64] = "New Block";
				static bool newBlockCollidable = true;
				static bool newBlockIsTransparent = false;
				static float newBlockColor[3] = { 0.5f, 0.5f, 0.5f };

                ImGui::InputText("Nazwa", nameBuf, 64);
                ImGui::Combo("Algorytm", &layerType, "Flat Fill\0Perlin Noise\0Perlin Noise 3D\0Simplex Noise\0");
                
                if (ImGui::InputInt("Start Y", &start_y)) {
                    if (start_y < 0) { start_y = 0; }
                    if (start_y > end_y) { start_y = end_y; }
                }
                if (ImGui::InputInt("End Y", &end_y)) {
                    if (end_y > 500) { end_y = 500; }
                    if (end_y < start_y) { end_y = start_y; }
                }

                ImGui::Separator();

                if (!isCreatingNewBlock) {
                    const std::vector<BlockData>& blockTypes = BlockDatabase::getAllBlocks();

                    if (currentBlockIndex >= blockTypes.size()) { currentBlockIndex = 1; }

                    if (ImGui::BeginCombo("Layer Block", blockTypes[currentBlockIndex].name.c_str())) {

                        for (int i = 0; i < blockTypes.size(); i++) {
							const bool isSelected = (currentBlockIndex == i);

                            ImVec4 col(blockTypes[i].color.x, blockTypes[i].color.y, blockTypes[i].color.z, 1.0f);

                            if (blockTypes[i].isTransparent) {
                                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.8f, 1.0f, 1.0f));
                                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
                            }
                            else {
                                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 1));
                                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                            }

                            ImGui::PushID(i);
                            ImGui::ColorButton("##color", col, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, ImVec2(15, 15));
                            ImGui::PopStyleVar();
                            ImGui::PopStyleColor();
                            
                            ImGui::SameLine();

                            if (ImGui::Selectable(blockTypes[i].name.c_str(), isSelected)) {
                                currentBlockIndex = i;
                            }

                            if (blockTypes[i].isTransparent) {
                                ImGui::SameLine();
                                ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "[T]");
                            }

							ImGui::PopID();

                            if (isSelected) { ImGui::SetItemDefaultFocus(); }
                        }

                        ImGui::Separator();
                        if (ImGui::Selectable("ADD")) {
                            isCreatingNewBlock = true;
                        }

                        ImGui::EndCombo();
                    }
                }
                else {

					ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1), "Creating new block...");
                    ImGui::Indent();

					ImGui::InputText("Block Name", newBlockName, 64);
					ImGui::ColorEdit3("Block Color", newBlockColor);
					ImGui::Checkbox("Transparent", &newBlockIsTransparent);

					bool isLimitReached = (BlockDatabase::getAllBlocks().size() >= 255);

                    if (isLimitReached) {
                        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Maximum number of block types (256) reached.");
                        ImGui::BeginDisabled();
                    }

                    if (ImGui::Button("Save Block")) {
						BlockID newID = BlockDatabase::registerBlockData(newBlockName, newBlockCollidable, newBlockIsTransparent, glm::vec3(newBlockColor[0], newBlockColor[1], newBlockColor[2]));
						
						currentBlockIndex = newID;
						isCreatingNewBlock = false;
                    }

                    if (isLimitReached) {
						ImGui::EndDisabled();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Cancel")) {
						isCreatingNewBlock = false;
                    }
                    ImGui::Unindent();
                }

                ImGui::Separator();

                if (isCreatingNewBlock) { ImGui::BeginDisabled(); }


                if (ImGui::Button("ADD", ImVec2(120, 0))) {
                    if (layerType == 0) {
                        layers.push_back(TerrainLayer(nameBuf, start_y, end_y, currentBlockIndex, std::make_unique<FlatFill>()));
                    }
                    else if(layerType == 1){
                        layers.push_back(TerrainLayer(nameBuf, start_y, end_y, currentBlockIndex, std::make_unique<PerlinNoise2D>(config.worldSeed, 0.05f, 1.0f, 4, 2.0f, 0.5f)));
                    }
                    else if (layerType == 2) {
                        layers.push_back(TerrainLayer(nameBuf, start_y, end_y, currentBlockIndex, std::make_unique<PerlinNoise3D>(config.worldSeed, 0.05f, 1.0f, 4, 2.0f, 0.5f)));
                    }
                    else if(layerType == 3){
                        layers.push_back(TerrainLayer(nameBuf, start_y, end_y, currentBlockIndex, std::make_unique<SimplexNoise>(config.worldSeed, 0.05f, 1.0f, 4, 2.0f, 0.5f)));
                    }
                    ImGui::CloseCurrentPopup();
                }

                if(isCreatingNewBlock) { ImGui::EndDisabled(); }

                ImGui::SameLine();

                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				    isCreatingNewBlock = false; 
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            static ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;
            if (ImGui::BeginTable("LayersTable", 4, tableFlags)) {
                ImGui::TableSetupColumn("On/Off", ImGuiTableColumnFlags_WidthFixed, 25.0f);
                ImGui::TableSetupColumn("Nazwa");
                ImGui::TableSetupColumn("Start Y");
                ImGui::TableSetupColumn("End Y");
                ImGui::TableHeadersRow();

                for (int i = 0; i < layers.size(); i++) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    ImGui::PushID(i);
                    ImGui::Checkbox("##enabled", &layers[i].isEnabled);
                    ImGui::PopID();

                    ImGui::TableNextColumn();
                    bool isSelected = (selectedLayerIndex == i);
                    std::string label = layers[i].name + "##" + std::to_string(i);

                    // Po kliknięciu wiersza zapamiętujemy indeks i odpalamy flagę przerzucenia zakładki
                    if (ImGui::Selectable(label.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
                        selectedLayerIndex = i;
                        
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                            activateDetailsTab = true;
                        }
                    }

                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                        ImGui::SetDragDropPayload("LAYER_ROW", &i, sizeof(int));
                        ImGui::Text("Przesun: %s", layers[i].name.c_str());
                        ImGui::EndDragDropSource();
                    }
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("LAYER_ROW")) {
                            int sourceIdx = *(const int*)payload->Data;
                            std::swap(layers[sourceIdx], layers[i]);
                            if (selectedLayerIndex == sourceIdx) selectedLayerIndex = i;
                            else if (selectedLayerIndex == i) selectedLayerIndex = sourceIdx;
                        }
                        ImGui::EndDragDropTarget();
                    }

                    ImGui::TableNextColumn();
                    ImGui::PushItemWidth(-FLT_MIN);
                    if(ImGui::InputInt(("##start" + std::to_string(i)).c_str(), &layers[i].startY, 0, 0)) {
                        if (layers[i].startY < 0) { layers[i].startY = 0; }
                        if (layers[i].startY > layers[i].endY) { layers[i].startY = layers[i].endY; }
                    }
                    ImGui::PopItemWidth();

                    ImGui::TableNextColumn();
                    ImGui::PushItemWidth(-FLT_MIN);
                    if(ImGui::InputInt(("##end" + std::to_string(i)).c_str(), &layers[i].endY, 0, 0)) {
                        if (layers[i].endY > 500) { layers[i].endY = 500; }
                        if (layers[i].endY < layers[i].startY) { layers[i].endY = layers[i].startY; }
                    }
                    ImGui::PopItemWidth();
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        // ZAKŁADKA 2: SZCZEGÓŁY ALGORYTMU
        if (selectedLayerIndex >= 0 && selectedLayerIndex < layers.size()) {

            // Jeśli flaga jest aktywna, wymuszamy focus na tę zakładkę
            ImGuiTabItemFlags tabFlags = 0;
            if (activateDetailsTab) {
                tabFlags |= ImGuiTabItemFlags_SetSelected;
                activateDetailsTab = false; // Resetujemy flagę
            }

            if (ImGui::BeginTabItem("Layer details", nullptr, tabFlags)) {
                TerrainLayer& currentLayer = layers[selectedLayerIndex];

                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Settings: %s", currentLayer.name.c_str());
                ImGui::Separator();

                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Block Type:");
                const std::vector<BlockData>& allBlocks = BlockDatabase::getAllBlocks();

                if (currentLayer.blockID < allBlocks.size()) {
                    if (ImGui::BeginCombo("##ChangeBlock", allBlocks[currentLayer.blockID].name.c_str())) {
                        for (int i = 0; i < allBlocks.size(); i++) {
                            bool isSelected = (currentLayer.blockID == i);
                            ImVec4 col(allBlocks[i].color.x, allBlocks[i].color.y, allBlocks[i].color.z, 1.0f);

                            if (allBlocks[i].isTransparent) {
                                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.8f, 1.0f, 1.0f));
                                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                            }
                            else {
                                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                            }

                            ImGui::ColorButton(std::to_string(i).c_str(), col, ImGuiColorEditFlags_NoTooltip);
                            ImGui::PopStyleVar();
                            ImGui::PopStyleColor();

                            ImGui::SameLine();
                            if (ImGui::Selectable(allBlocks[i].name.c_str(), isSelected)) {
                                currentLayer.blockID = i;
                            }

                            if (allBlocks[i].isTransparent) {
                                ImGui::SameLine();
                                ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "[T]");
                            }
                        }
                        ImGui::EndCombo();
                    }
                }

                ImGui::Spacing();
                
                const char* blendNames[] = { "NORMAL", "ADD", "SUBTRACT", "MULTIPLY", "MAX", "MIN", "SMOOTH" , "ABSOLUTE", "CARVE", "CARVEIN"};
                ImGui::Combo("Blending Mode", (int*)&currentLayer.blendMode, blendNames, IM_ARRAYSIZE(blendNames));

                if (currentLayer.blendMode == BlendMode::SMOOTH || currentLayer.blendMode == BlendMode::CARVE) {
                    ImGui::SliderFloat("Blending Weight", &currentLayer.blendWeight, 0.0f, 1.0f);
                }

                if (currentLayer.blendMode == BlendMode::CARVEIN) {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Target Block (Carve ONLY in this):");

                    if (currentLayer.targetBlockID < allBlocks.size()) {
                        if (ImGui::BeginCombo("##TargetBlock", allBlocks[currentLayer.targetBlockID].name.c_str())) {

                            for (int i = 0; i < allBlocks.size(); i++) {
                                bool isSelected = (currentLayer.targetBlockID == i);
                                ImVec4 col(allBlocks[i].color.x, allBlocks[i].color.y, allBlocks[i].color.z, 1.0f);

                                ImGui::PushID(i + 1000);
                                ImGui::ColorButton("##targetColor", col, ImGuiColorEditFlags_NoTooltip, ImVec2(15, 15));
                                ImGui::PopID();

                                ImGui::SameLine();
                                if (ImGui::Selectable(allBlocks[i].name.c_str(), isSelected)) {
                                    currentLayer.targetBlockID = static_cast<BlockID>(i);
                                }

                                if (isSelected) ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                    }
                }

                ImGui::Spacing();
                ImGui::Separator();

                if (currentLayer.algorithm) {
                    currentLayer.algorithm->renderImGui();
                }

                ImGui::Spacing();
                ImGui::Separator();

                ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Mathematical Modifiers (%zu/6)", currentLayer.activeModifiers.size());
                ImGui::Spacing();

                for (int i = 0; i < currentLayer.activeModifiers.size(); i++) {
                    ImGui::PushID(static_cast<int>(i));
                    ImGui::BeginGroup();

                    currentLayer.activeModifiers[i]->renderImGui();

                    ImGui::SameLine();
                    if (ImGui::Button("X", ImVec2(24, 24))) {
                        currentLayer.activeModifiers.erase(currentLayer.activeModifiers.begin() + i);
                    }

                    ImGui::EndGroup();
                    ImGui::PopID();
                }

                ImGui::Spacing();

                if (currentLayer.activeModifiers.size() >= 6) {
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Maximum number od modifiers (6) reached.");
                }
                else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.5f, 0.25f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.6f, 0.3f, 1.0f));

                    if (ImGui::Button("Add Modifier", ImVec2(-FLT_MIN, 30))) {
                        ImGui::OpenPopup("AddModifierMenu");
                    }
                    ImGui::PopStyleColor(2);

                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(9, 9));
                    if (ImGui::BeginPopup("AddModifierMenu")) {
                        ImGui::TextDisabled("Select Modifier Type:");
                        ImGui::Separator();

                        if (ImGui::Selectable("Invert")) { currentLayer.activeModifiers.push_back(std::make_unique<ModifierInvert>()); }
                        if (ImGui::Selectable("Power")) { currentLayer.activeModifiers.push_back(std::make_unique<ModifierPower>()); }
                        if (ImGui::Selectable("Terrace")) { currentLayer.activeModifiers.push_back(std::make_unique<ModifierTerrace>()); }
                        if (ImGui::Selectable("Ridged")) { currentLayer.activeModifiers.push_back(std::make_unique<ModifierRidged>()); }
                        if (ImGui::Selectable("Mesa Curve")) { currentLayer.activeModifiers.push_back(std::make_unique<ModifierMesaCurve>()); }

                        ImGui::EndPopup();
                    }

                    ImGui::PopStyleVar();
                }

                ImGui::EndTabItem();
            }
        }

        // ZAKŁADKA 3: KAMERA
        if (ImGui::BeginTabItem("Camera")) {

            ImGui::SliderFloat("FOV", &config.fov, 80.0f, 110.0f);
            ImGui::SliderFloat("Camera Speed", &config.cameraSpeed, 1.0f, 200.0f);
            ImGui::SliderFloat("View Distance", &config.viewDistance, 5.0f, 3000.0f);
            ImGui::EndTabItem();
        }

        // ZAKŁADKA 4: ZAAWANSOWANE
        if (ImGui::BeginTabItem("Advanced")) {

            ImGui::Checkbox("Frustum Culling", &config.isFrustumCullingEnabled);
            ImGui::Checkbox("Hidden Face Culling", &config.isHiddenWallCullingEnabled);
            ImGui::Checkbox("Show ChunkColumn Borders", &config.showChunkColumnsBorder);
            ImGui::Checkbox("Show Crosshair", &config.isCrosshairEnabled);
            ImGui::Checkbox("Show Coords HUD", &config.isCoordsHudEnabled);
            ImGui::Checkbox("Show Chunk Stats HUD", &config.isChunkHudEnabled);
            ImGui::Checkbox("Transparent Block Camera Filter", &config.isTransparentBlockCameraFilterEnabled);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    if (isLoading) { ImGui::BeginDisabled(); }

    if (ImGui::Button("REGENERATE WORLD", ImVec2(-FLT_MIN, 30))) {
		world->regenerateWorld();
		SceneManager::getInstance().pushScene(std::make_unique<LoadingScene>(world));
    }

    if (isLoading) { ImGui::EndDisabled(); }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    ImGui::End();

}