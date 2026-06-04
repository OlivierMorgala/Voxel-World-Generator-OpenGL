#pragma once
#include "scenes/Scene.h"
#include "world/World.h"
#include "world/WorldTerrainGenerator.h"
#include <memory>

class LoadingScene : public Scene
{
private:
	World* world;
public:
	LoadingScene(World* loadedWorld);
	~LoadingScene() override = default;

	void onEnter() override;
	void onExit() override;
	void onUpdate(float deltaTime) override;
	void render() override;
	void onImGuiRender() override;
};

