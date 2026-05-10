#include "world/WorldTerrainGenerator.h"

void WorldTerrainGenerator::applyToColumn(ChunkColumn& column) {
    for (auto& layer : generationLayers) {
        layer->applyToColumn(column);
    }
}

void WorldTerrainGenerator::clearLayers() {
    generationLayers.clear();
}