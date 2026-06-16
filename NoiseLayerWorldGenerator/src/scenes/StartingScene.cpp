#include "scenes/StartingScene.h"
#include "scenes/WorldGeneratorScene.h" 
#include "managers/SceneManager.h"      
#include "managers/TextureManager.h"  
#include "world\generationAlgorithms\PerlinNoise2D.h"
#include "world\generationAlgorithms\FlatFill.h"
#include <imgui.h>
#include <iostream>
#include <memory>
#include <algorithm>

StartingScene::StartingScene() : backgroundTextureID(0)
{
}

void StartingScene::onEnter()
{
    std::cout << "+[Scene] LOADED: StartingScene" << std::endl;

    // Przywracamy kursor, bo jesteśmy w menu
    if (window) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    backgroundTextureID = TextureManager::loadTexture("assets/textures/planet-sheet.png");
}

void StartingScene::onExit()
{
    std::cout << "-[Scene] UNLOADED: StartingScene" << std::endl;

    if (backgroundTextureID != 0) {
        glDeleteTextures(1, &backgroundTextureID);
    }
}

void StartingScene::onUpdate(float deltaTime)
{
    // Aktualizacja klatek animacji na start ekranie
    animationTimer += deltaTime;
    if (animationTimer >= frameDuration) {
        animationTimer -= frameDuration;
        currentFrame = (currentFrame + 1) % totalFrames;
    }
}

void StartingScene::render()
{
    // Kosmiczne, granatowe tło menu
    glClearColor(0.03f, 0.03f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void StartingScene::onImGuiRender()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    // Okno niewidzialne, trzymające sam layout
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin("StartingScreen", nullptr, windowFlags)) {

        ImVec2 center = ImVec2(viewport->Size.x * 0.5f, viewport->Size.y * 0.5f);

        // Ustalanie wymiarów UI i wyśrodkowanie
        float planetSize = 240.0f;
        float buttonWidth = 450.0f;
        float buttonHeight = 70.0f;
        float spacing = 18.0f;
        float sectionSpacing = 50.0f;

        float buttonsTotalHeight = (buttonHeight * 4) + (spacing * 3);
        float totalHeight = planetSize + sectionSpacing + buttonsTotalHeight;

        float startY = center.y - totalHeight * 0.5f;

        // Logo - tekst
        ImGui::SetWindowFontScale(3.8f);
        const char* gameTitle = "TERRABOXEL";
        ImVec2 titleSize = ImGui::CalcTextSize(gameTitle);

        float titleAndPlanetWidth = planetSize + 40.0f + titleSize.x;

        ImGui::SetCursorPosY(startY);
        ImGui::SetCursorPosX(center.x - titleAndPlanetWidth * 0.5f);

        // Logo - animowana planeta z ze sprite-sheeta
        if (backgroundTextureID != 0) {
            int currentCol = currentFrame % cols;
            int currentRow = currentFrame / cols;

            float u_step = 1.0f / static_cast<float>(cols);
            float v_step = 1.0f / static_cast<float>(rows);

            float u0 = currentCol * u_step;
            float u1 = u0 + u_step;
            float v0 = 1.0f - (currentRow * v_step);
            float v1 = v0 - v_step;

            ImGui::Image((void*)(intptr_t)backgroundTextureID, ImVec2(planetSize, planetSize), ImVec2(u0, v0), ImVec2(u1, v1));
        }
        else {
            ImGui::Dummy(ImVec2(planetSize, planetSize));
        }

        ImGui::SameLine(0.0f, 40.0f);
        float textPosY = startY + (planetSize - titleSize.y) * 0.5f;
        ImGui::SetCursorPosY(textPosY);
        ImGui::TextColored(ImVec4(0.9f, 0.95f, 1.0f, 1.0f), "%s", gameTitle);

        // --- PRZYCISKI MENU ---
        ImGui::SetWindowFontScale(1.35f);
        ImGui::SetCursorPosY(startY + planetSize + sectionSpacing);

        // Zaokrąglenia i szare kolory dla standardowych presetów
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.15f, 0.17f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.22f, 0.25f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.88f, 0.90f, 0.93f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.30f, 0.32f, 0.35f, 0.40f));

        std::vector<TerrainLayer> preset;

        // Przyciski przekazujące różną listę warstw (Preset) do właściwej gry
        ImGui::SetCursorPosX(center.x - buttonWidth * 0.5f);
        if (ImGui::Button("Mountain Valley Preset", ImVec2(buttonWidth, buttonHeight))) {
            preset.clear();
            SceneManager::getInstance().pushScene(std::make_unique<WorldGeneratorScene>(std::move(preset)));
        }

        ImGui::SetCursorPosX(center.x - buttonWidth * 0.5f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spacing);
        if (ImGui::Button("Desert Dunes Preset", ImVec2(buttonWidth, buttonHeight))) {
            preset.clear();
            SceneManager::getInstance().pushScene(std::make_unique<WorldGeneratorScene>(std::move(preset)));
        }

        ImGui::SetCursorPosX(center.x - buttonWidth * 0.5f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spacing);
        if (ImGui::Button("Frozen Peaks Preset", ImVec2(buttonWidth, buttonHeight))) {
            preset.clear();

            // Ręczny kod presetu np. dla ośnieżonych gór (High Peaks)
            TerrainLayer peaks("High Peaks", 20, 110, 1, std::make_unique<PerlinNoise2D>(456, 0.08f, 0.9f, 6, 2.2f, 0.6f));
            peaks.blendMode = BlendMode::NORMAL;
            preset.push_back(std::move(peaks));

            SceneManager::getInstance().pushScene(std::make_unique<WorldGeneratorScene>(std::move(preset)));
        }

        ImGui::PopStyleColor(5);

        // Tryb customowy wyróżniony na zielono
        ImGui::SetCursorPosX(center.x - buttonWidth * 0.5f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spacing * 1.8f);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.32f, 0.24f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.42f, 0.32f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.24f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.95f, 0.92f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.32f, 0.52f, 0.40f, 0.45f));

        if (ImGui::Button("Custom World Generator", ImVec2(buttonWidth, buttonHeight))) {
            SceneManager::getInstance().pushScene(std::make_unique<WorldGeneratorScene>());
        }

        ImGui::PopStyleColor(5);
        ImGui::PopStyleVar(2);

        // Skala wraca do normy
        ImGui::SetWindowFontScale(1.0f);
    }
    ImGui::End();
}
