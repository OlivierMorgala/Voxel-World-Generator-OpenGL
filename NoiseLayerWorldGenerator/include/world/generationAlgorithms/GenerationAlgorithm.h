#pragma once
#include <string>
#include <imgui.h>
#include <world/ChunkColumn.h>

class GenerationAlgorithm
{
public:
	std::string layerName;

	int startY;
	int endY;

	GenerationAlgorithm(std::string layerName, int startY, int endY);
	virtual ~GenerationAlgorithm() = default;

	virtual void applyToColumn(ChunkColumn& column) = 0;

	virtual void renderImGuiSettings() = 0;
};

