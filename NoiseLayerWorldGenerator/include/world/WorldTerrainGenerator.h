#pragma once
#include <vector>
#include "world/ChunkColumn.h"
#include "world/TerrainPipeline.h"

class WorldTerrainGenerator
{
private:

public:
	//wektor obiketów ktore s¹ ró¿nymi algorytmami generacji
	std::vector<TerrainLayer> generationLayers;

	//Wywo³anie algorytmów na renderowanej kolumnie
	void applyToColumn(ChunkColumn& column);

	//Metoda czyszcz¹ca obecne warstwy
	void clearLayers();
};

