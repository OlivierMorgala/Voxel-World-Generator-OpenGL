#include "gui/WorldGeneratorUI.h"
#include "world/generationAlgorithms/PerlinNoise2D.h"
#include "world/generationAlgorithms/FlatFill.h"
#include <world/WorldConfig.h>
#include <world/World.h>
#include <managers/SceneManager.h>
#include <scenes/LoadingScene.h>

WorldGeneratorUI::WorldGeneratorUI(WorldTerrainGenerator* generator, World* world) :
    worldGenerator(generator),
    world(world),
    selectedLayerIndex(-1)
{

}

void WorldGeneratorUI::renderImGui()
{
    if (!worldGenerator) return;

    // Obliczanie wymiarów i pozycji panelu
	bool isLoading = (world->getCurrentState() == WorldState::LOADING);

    float screenWidth = ImGui::GetIO().DisplaySize.x;
    float screenHeight = ImGui::GetIO().DisplaySize.y;
    float panelWidth = screenWidth / 4.0f;

    ImGui::SetNextWindowPos(ImVec2(screenWidth - panelWidth, 0));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, screenHeight));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.05f, 0.85f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("World Generator Settings", nullptr, windowFlags);

    // Rysowanie jasnej linii po lewej stronie
    ImVec2 p_min = ImGui::GetWindowPos();
    ImVec2 p_max = ImVec2(p_min.x + 3.0f, p_min.y + screenHeight);
    ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, IM_COL32(100, 255, 100, 255));

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);

    ImGui::TextColored(ImVec4(1, 1, 1, 1), "Ustawienia Generatora");
    ImGui::Separator();

    static int globalSeed = 12345;
    ImGui::InputInt("Global Seed", &globalSeed);
    ImGui::Spacing();

    auto& layers = worldGenerator->generationLayers;

    // Flaga do automatycznego focusowania zakładki ze szczegółami
    static bool activateDetailsTab = false;

    // Początek paska zakładek
    if (ImGui::BeginTabBar("GeneratorTabs")) {

        // ZAKŁADKA 1: WARSTWY
        if (ImGui::BeginTabItem("Warstwy")) {

            if (ImGui::Button("+", ImVec2(30, 0))) {
                ImGui::OpenPopup("Dodaj warstwe");
            }
            ImGui::SameLine();
            if (ImGui::Button("-", ImVec2(30, 0)) && selectedLayerIndex >= 0 && selectedLayerIndex < layers.size()) {
                layers.erase(layers.begin() + selectedLayerIndex);
                selectedLayerIndex = -1;
            }

            if (ImGui::BeginPopupModal("Dodaj warstwe", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                static char nameBuf[64] = "Nowa warstwa";
                static int layerType = 0;
                static int start_y = 0, end_y = 64;

                static int currentBlockIndex = 1;
				static bool isCreatingNewBlock = false;

				static char newBlockName[64] = "New Block";
				static bool newBlockCollidable = true;
				static bool newBlockIsTransparent = false;
				static float newBlockColor[3] = { 0.5f, 0.5f, 0.5f };

                ImGui::InputText("Nazwa", nameBuf, 64);
                ImGui::Combo("Algorytm", &layerType, "Perlin Noise\0Flat Fill\0");
                ImGui::InputInt("Start Y", &start_y);
                ImGui::InputInt("End Y", &end_y);
                ImGui::Separator();

                if (!isCreatingNewBlock) {
                    const auto& blockTypes = BlockDatabase::getAllBlocks();

                    if (currentBlockIndex >= blockTypes.size()) { currentBlockIndex = 1; }

                    if (ImGui::BeginCombo("Layer Block", blockTypes[currentBlockIndex].name.c_str())) {

                        for (int i = 1; i < blockTypes.size(); i++) {
							const bool isSelected = (currentBlockIndex == i);

                            ImVec4 col(blockTypes[i].color.x, blockTypes[i].color.y, blockTypes[i].color.z, 1.0f);

							ImGui::PushID(i);

							ImGui::ColorButton("##color", col, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, ImVec2(15, 15));
                            ImGui::SameLine();

                            if (ImGui::Selectable(blockTypes[i].name.c_str(), isSelected)) {
                                currentBlockIndex = i;
                            }

							ImGui::PopID();

                            if (isSelected) { ImGui::SetItemDefaultFocus(); }
                        }

                        ImGui::Separator();
                        if (ImGui::Selectable("+ADD+")) {
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
					ImGui::Checkbox("Collidable", &newBlockCollidable);
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


                if (ImGui::Button("Dodaj", ImVec2(120, 0))) {
                    if (layerType == 0) {
                        layers.push_back(std::make_unique<PerlinNoise2D>(nameBuf, start_y, end_y, globalSeed, currentBlockIndex));
                    }
                    else if (layerType == 1) {
                        layers.push_back(std::make_unique<FlatFill>(nameBuf, start_y, end_y, currentBlockIndex));
                    }
                    ImGui::CloseCurrentPopup();
                }

                if(isCreatingNewBlock) { ImGui::EndDisabled(); }

                ImGui::SameLine();
                if (ImGui::Button("Anuluj", ImVec2(120, 0))) {
				    isCreatingNewBlock = false; 
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            static ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;
            if (ImGui::BeginTable("LayersTable", 3, tableFlags)) {
                ImGui::TableSetupColumn("Nazwa");
                ImGui::TableSetupColumn("Start Y");
                ImGui::TableSetupColumn("End Y");
                ImGui::TableHeadersRow();

                for (int i = 0; i < layers.size(); i++) {
                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    bool isSelected = (selectedLayerIndex == i);
                    std::string label = layers[i]->layerName + "##" + std::to_string(i);

                    // Po kliknięciu wiersza zapamiętujemy indeks i odpalamy flagę przerzucenia zakładki
                    if (ImGui::Selectable(label.c_str(), isSelected)) {
                        selectedLayerIndex = i;
                        activateDetailsTab = true;
                    }

                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                        ImGui::SetDragDropPayload("LAYER_ROW", &i, sizeof(int));
                        ImGui::Text("Przesun: %s", layers[i]->layerName.c_str());
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
                    ImGui::InputInt(("##start" + std::to_string(i)).c_str(), &layers[i]->startY, 0, 0);
                    ImGui::PopItemWidth();

                    ImGui::TableNextColumn();
                    ImGui::PushItemWidth(-FLT_MIN);
                    ImGui::InputInt(("##end" + std::to_string(i)).c_str(), &layers[i]->endY, 0, 0);
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

            if (ImGui::BeginTabItem("Szczegoly algorytmu", nullptr, tabFlags)) {
                ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), "Ustawienia: %s", layers[selectedLayerIndex]->layerName.c_str());
                ImGui::Separator();
                ImGui::Spacing();

                // Tutaj modyfikujesz parametry (częstotliwość, oktawy, itd.)
                layers[selectedLayerIndex]->renderImGuiSettings();

                ImGui::EndTabItem();
            }
        }

        // ZAKŁADKA 3: KAMERA
        if (ImGui::BeginTabItem("Kamera")) {
            static float fov = 90.0f;
            static float speed = 5.0f;
            static bool noClip = true;
            ImGui::SliderFloat("FOV", &fov, 30.0f, 120.0f);
            ImGui::SliderFloat("Predkosc", &speed, 1.0f, 50.0f);
            ImGui::Checkbox("NoClip", &noClip);
            ImGui::EndTabItem();
        }

        // ZAKŁADKA 4: ZAAWANSOWANE
        if (ImGui::BeginTabItem("Zaawansowane")) {
            static bool frustumCulling = true;
            static bool hiddenFaceCulling = true;
            static bool greedyMeshing = true;
            ImGui::Checkbox("Frustum Culling", &frustumCulling);
            ImGui::Checkbox("Hidden Face Culling", &hiddenFaceCulling);
            ImGui::Checkbox("Greedy Meshing", &greedyMeshing);
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