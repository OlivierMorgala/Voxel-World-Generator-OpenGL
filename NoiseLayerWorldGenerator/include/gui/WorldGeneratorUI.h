#pragma once
#include "world/generationAlgorithms/GenerationAlgorithm.h"
#include <vector>
#include <memory>

class WorldGeneratorUI
{
private:
	std::vector<std::unique_ptr<GenerationAlgorithm>> generationLayers;
	int selectedLayerIndex = -1;

public:
	WorldGeneratorUI();

	void renderImGui();

	void generateColumn();
};

