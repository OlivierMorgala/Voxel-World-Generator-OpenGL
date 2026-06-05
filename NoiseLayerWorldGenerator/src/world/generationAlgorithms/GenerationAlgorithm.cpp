#include "world/generationAlgorithms/GenerationAlgorithm.h"

GenerationAlgorithm::GenerationAlgorithm(std::string layerName, int startY, int endY, BlockID blockID) :
layerName(layerName), startY(startY), endY(endY), layerBlockID(blockID)
{
		layerID = idCounter++;
}
