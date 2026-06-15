#pragma once
#include <string>
#include <vector>
#include <imgui.h>
#include "world/BlockType.h"

enum class BlendMode {
	NORMAL,
	ADD,
	SUBTRACT,
	MULTIPLY,
	MAX,
	MIN,
	SMOOTH,
	ABSOLUTE,
	CARVE
};

class TerrainModifier
{
public:
	virtual ~TerrainModifier() = default;

	virtual float modify(float value) = 0;
	virtual void renderImGui() = 0;
};

class TerrainAlgorithm {
public:
	virtual ~TerrainAlgorithm() = default;

	virtual float evaluate(float x, float z) = 0;
	virtual float evaluate3D(float x, float y, float z) { return 0.0f; };

	virtual void renderImGui() = 0;
	virtual void setSeed(int newSeed) = 0;
};

class TerrainLayer {
public:
	std::string name;
	bool isEnabled = true;

	BlendMode blendMode = BlendMode::ABSOLUTE;
	float blendWeight = 0.5f; //Tylko dla SMOOTH

	int startY;
	int endY;
	BlockID blockID;

	std::unique_ptr<TerrainAlgorithm> algorithm;
	std::vector<std::unique_ptr<TerrainModifier>> activeModifiers;

	TerrainLayer(std::string name, int startY, int endY, BlockID blockID, std::unique_ptr<TerrainAlgorithm> algorithm);

};

