#include "gui/WorldGeneratorUI.h"
#include "world/generationAlgorithms/PerlinNoise2D.h"
#include "world/generationAlgorithms/FlatFill.h"
#include <world/WorldConfig.h>
#include <world/World.h>
#include <memory>   // Wymagane dla std::make_unique i std::unique_ptr
#include <utility>  // Wymagane dla std::swap
#include <string>   

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

                ImGui::InputText("Nazwa", nameBuf, 64);
                ImGui::Combo("Algorytm", &layerType, "Perlin Noise\0Flat Fill\0");
                ImGui::InputInt("Start Y", &start_y);
                ImGui::InputInt("End Y", &end_y);

                if (ImGui::Button("Dodaj", ImVec2(120, 0))) {
                    if (layerType == 0) {
                        layers.push_back(std::make_unique<PerlinNoise2D>(nameBuf, start_y, end_y, globalSeed));
                    }
                    else if (layerType == 1) {
                        layers.push_back(std::make_unique<FlatFill>(nameBuf, start_y, end_y));
                    }
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Anuluj", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
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
                    if (ImGui::Selectable(label.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
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