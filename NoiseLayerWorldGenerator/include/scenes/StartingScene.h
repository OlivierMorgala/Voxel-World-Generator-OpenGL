#pragma once
#include "scenes/Scene.h"

class StartingScene : public Scene
{
private:
    unsigned int backgroundTextureID = 0;

    // Zmienne do obsługi animacji obracającej się planety
    int cols = 5;
    int rows = 24;
    int totalFrames = 120;
    int currentFrame = 0;
    float animationTimer = 0.0f;

    float frameDuration = 0.08f;

public:
    StartingScene();
    ~StartingScene() override = default;

    void onEnter() override;
    void onExit() override;
    void onUpdate(float deltaTime) override;
    void render() override;
    void onImGuiRender() override;
};