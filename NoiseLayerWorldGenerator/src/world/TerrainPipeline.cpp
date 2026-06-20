#include "world/TerrainPipeline.h"

// Konstruktor Klasy TerrainLayer -> inicjalizuje najwazniesze zmienne
TerrainLayer::TerrainLayer(std::string name, int startY, int endY, BlockID blockID, std::unique_ptr<TerrainAlgorithm> algorithm)
	: name(name), startY(startY), endY(endY), blockID(blockID), algorithm(std::move(algorithm)) {

}