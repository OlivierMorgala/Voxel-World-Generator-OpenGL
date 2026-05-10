#pragma once
#include "world/WorldTerrainGenerator.h"
#include <vector>
#include <memory>

class WorldGeneratorUI
{
private:
	WorldTerrainGenerator* worldGenerator = nullptr;
	int selectedLayerIndex = -1;

public:
	WorldGeneratorUI(WorldTerrainGenerator* generator);

	void renderImGui();
};

