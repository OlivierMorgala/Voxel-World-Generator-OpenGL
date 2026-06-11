#pragma once
#include "world/TerrainPipeline.h"

class FlatFill : public TerrainAlgorithm
{
private:

public:
	FlatFill() = default;

	float evaluate(float x, float z) override;
	void renderImGui() override;
};

