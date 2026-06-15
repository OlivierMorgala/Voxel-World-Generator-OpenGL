#pragma once
#include "scenes/Scene.h"
#include "world/World.h"
#include "world/WorldTerrainGenerator.h"
#include <memory>

class LoadingScene : public Scene
{
private:
	World* world;

	//zmienne dla gifu
	unsigned int planetTextureID = 0;
	int cols = 5;
	int rows = 24;
	int totalFrames = 120;

	int currentFrame = 0;
	float animationTimer = 0.0f;
	float frameDuration = 0.03f;

public:
	LoadingScene(World* loadedWorld);
	~LoadingScene() override = default;

	void onEnter() override;
	void onExit() override;
	void onUpdate(float deltaTime) override;
	void render() override;
	void onImGuiRender() override;
};

