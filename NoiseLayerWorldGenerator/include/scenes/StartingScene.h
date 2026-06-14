#pragma once
#include "scenes/Scene.h"

class StartingScene : public Scene
{
private:
    unsigned int backgroundTextureID = 0; // ID tekstury z OpenGL

    // Zmienne do obsługi animacji obracającej się planety (skopiowane z LoadingScene)
    int cols = 5;
    int rows = 24;
    int totalFrames = 120;
    int currentFrame = 0;
    float animationTimer = 0.0f;
    float frameDuration = 0.03f;

public:
    StartingScene();
    ~StartingScene() override = default;

    void onEnter() override;
    void onExit() override;
    void onUpdate(float deltaTime) override;
    void render() override;
    void onImGuiRender() override;
};