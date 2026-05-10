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

public:
	WorldGeneratorUI(WorldTerrainGenerator* generator, World* world);

	void renderImGui();
};

