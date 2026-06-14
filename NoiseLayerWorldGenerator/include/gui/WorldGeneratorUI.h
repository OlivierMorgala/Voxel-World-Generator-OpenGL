#pragma once
#include "world/WorldTerrainGenerator.h"
#include <vector>
#include <memory>

class World;

class WorldGeneratorUI
{
private:
	WorldTerrainGenerator* worldGenerator;
	World* world;
	int selectedLayerIndex = -1;
	float openAnimationProgress = 0.0f;


public:
	WorldGeneratorUI(WorldTerrainGenerator* generator, World* world);

	void renderImGui(bool isMenuOpen);
};

