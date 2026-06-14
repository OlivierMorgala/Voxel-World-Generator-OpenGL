#include "scenes/StartingScene.h"
#include "scenes/WorldGeneratorScene.h" 
#include "managers/SceneManager.h"      
#include "managers/TextureManager.h"    
#include <imgui.h>
#include <iostream>
#include <memory>
#include <algorithm> // Do std::min

StartingScene::StartingScene() : backgroundTextureID(0)
{
}

void StartingScene::onEnter()
{
    std::cout << "+[Scene] LOADED: StartingScene" << std::endl;

    if (window) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    // Ładujemy plik z atlasem obracającej się planety
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
    // Aktualizacja klatek animacji planety dokładnie tak jak w LoadingScene
    animationTimer += deltaTime;
    if (animationTimer >= frameDuration) {
        animationTimer -= frameDuration;
        currentFrame = (currentFrame + 1) % totalFrames;
    }
}

void StartingScene::render()
{
    // Głębokie, bardzo ciemne tło (prawie czarne), które idealnie współgra z kosmosem/planetą
    glClearColor(0.03f, 0.03f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void StartingScene::onImGuiRender()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin("StartingScreen", nullptr, windowFlags)) {

        ImVec2 center = ImVec2(viewport->Size.x * 0.5f, viewport->Size.y * 0.5f);

        // 1. OBLICZANIE KLATKI ANIMACJI I RYSOWANIE JEDNEJ OBRACAJĄCEJ SIĘ PLANETY
        if (backgroundTextureID != 0) {
            // Logika wycinania pojedynczej klatki z atlasu (z LoadingScene)
            int currentCol = currentFrame % cols;
            int currentRow = currentFrame / cols;

            float u_step = 1.0f / static_cast<float>(cols);
            float v_step = 1.0f / static_cast<float>(rows);

            float u0 = currentCol * u_step;
            float u1 = u0 + u_step;
            float v0 = 1.0f - (currentRow * v_step);
            float v1 = v0 - v_step;

            // Ustalamy rozmiar planety tak, aby była proporcjonalna i się nie rozciągała (np. 65% mniejszego wymiaru ekranu)
            float planetSize = std::min(viewport->Size.x, viewport->Size.y) * 0.65f;

            // Pozycja planety (wyśrodkowana na ekranie)
            ImVec2 p_min = ImVec2(center.x - planetSize * 0.5f, center.y - planetSize * 0.5f);
            ImVec2 p_max = ImVec2(center.x + planetSize * 0.5f, center.y + planetSize * 0.5f);

            // Rysujemy tylko wyciętą, pojedynczą klatkę
            ImGui::GetBackgroundDrawList()->AddImage(
                (void*)(intptr_t)backgroundTextureID,
                p_min,
                p_max,
                ImVec2(u0, v0),
                ImVec2(u1, v1)
            );
        }

        // 2. USTARIENIA WYMIARÓW I POZYCJI PANELI/PRZYCISKÓW
        float buttonWidth = 350.0f;
        float buttonHeight = 55.0f;
        float spacing = 15.0f;

        float totalHeight = (buttonHeight * 4) + (spacing * 3) + 20.0f;
        ImGui::SetCursorPos(ImVec2(center.x - buttonWidth * 0.5f, center.y - totalHeight * 0.5f));

        // Zaokrąglenia i delikatne obramowania
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

        // KOLORY DLA PRZYCISKÓW PRESETÓW (Stonowany, elegancki ciemny grafit / półprzezroczysty)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.15f, 0.17f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.22f, 0.25f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.88f, 0.90f, 0.93f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.30f, 0.32f, 0.35f, 0.40f));

        // PRZYCISK 1
        if (ImGui::Button("Predefiniowana Scena: Gorska Dolina", ImVec2(buttonWidth, buttonHeight))) {
            SceneManager::getInstance().pushScene(std::make_unique<WorldGeneratorScene>());
        }

        ImGui::SetCursorPosX(center.x - buttonWidth * 0.5f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spacing);

        // PRZYCISK 2
        if (ImGui::Button("Predefiniowana Scena: Pustynne Wydmy", ImVec2(buttonWidth, buttonHeight))) {
            SceneManager::getInstance().pushScene(std::make_unique<WorldGeneratorScene>());
        }

        ImGui::SetCursorPosX(center.x - buttonWidth * 0.5f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spacing);

        // PRZYCISK 3
        if (ImGui::Button("Predefiniowana Scena: Zimowy Szczyt", ImVec2(buttonWidth, buttonHeight))) {
            SceneManager::getInstance().pushScene(std::make_unique<WorldGeneratorScene>());
        }

        // Usuwamy grafitowe kolory przed narysowaniem zielonego przycisku
        ImGui::PopStyleColor(5);

        ImGui::SetCursorPosX(center.x - buttonWidth * 0.5f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spacing * 2.0f); // Większy odstęp dla wyróżnienia

        // KOLORY DLA CZWARTEGO PRZYCISKU (Stonowana, głęboka zieleń szmaragdowa/oliwkowa)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.32f, 0.24f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.42f, 0.32f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.24f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.95f, 0.92f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.32f, 0.52f, 0.40f, 0.45f));

        // PRZYCISK 4: WŁASNA SCENA
        if (ImGui::Button("WLASNA SCENA (Kreator)", ImVec2(buttonWidth, buttonHeight))) {
            SceneManager::getInstance().pushScene(std::make_unique<WorldGeneratorScene>());
        }

        // Czyszczenie stylów
        ImGui::PopStyleColor(5);
        ImGui::PopStyleVar(2);
    }
    ImGui::End();
}