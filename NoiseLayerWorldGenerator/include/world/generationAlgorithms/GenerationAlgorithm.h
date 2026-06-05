#pragma once
#include <string>
#include <imgui.h>
#include <world/ChunkColumn.h>

class GenerationAlgorithm
{
private:
	static inline unsigned int idCounter = 1;
public:
	std::string layerName;

	unsigned int layerID;
	int startY;
	int endY;
	BlockID layerBlockID;

	GenerationAlgorithm(std::string layerName, int startY, int endY, BlockID blockID);
	virtual ~GenerationAlgorithm() = default;

	virtual void applyToColumn(ChunkColumn& column) = 0;

	virtual void renderImGuiSettings() = 0;
};

