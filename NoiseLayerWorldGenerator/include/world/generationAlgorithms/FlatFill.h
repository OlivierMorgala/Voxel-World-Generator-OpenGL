#pragma once
#include "world/TerrainPipeline.h"

// FlatFill odpowiada za generowanie totalnie plaskiego podloza w swiecie

class FlatFill : public TerrainAlgorithm
{
private:

public:
	FlatFill() = default;

	
	float evaluate(float x, float z) override;
	float evaluate3D(float x, float y, float z) override;
	void renderImGui() override;
	void setSeed(int newSeed) override;
};

