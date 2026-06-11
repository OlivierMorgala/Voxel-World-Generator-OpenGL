#pragma once
#include <vector>
#include <string>
#include "world/TerrainPipeline.h"

class PerlinNoise2D : public TerrainAlgorithm {
private:
	float frequency;
	float amplitude;
	int octaves;
	float frequencyChange;
	float amplitudeChange;
	std::vector<int> permutations;

	float smoothInterpolation(float t, float x, float y);
	float dotProduct(float gx, float gy, float x, float y);
	float perlinNoiseFunction(float x, float y);

public:
	PerlinNoise2D(unsigned seed, float freq, float amp, int octav, float freqChange, float ampChange);

	float evaluate(float x, float z) override;
	void renderImGui() override;
};