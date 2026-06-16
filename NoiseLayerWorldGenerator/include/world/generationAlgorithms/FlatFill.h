#pragma once
#include "world/TerrainPipeline.h"

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

