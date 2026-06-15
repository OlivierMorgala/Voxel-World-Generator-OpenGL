#include "world/WorldTerrainGenerator.h"
#include "world/Chunk.h"
#include "world/WorldConfig.h"
#include <algorithm>

void WorldTerrainGenerator::applyToColumn(ChunkColumn& column) {
    int worldX = column.getX() * Chunk::CHUNK_SIZE;
    int worldZ = column.getZ() * Chunk::CHUNK_SIZE;

    int maxWorldHeight = config.worldHeightInChunks * Chunk::CHUNK_SIZE - 1;

    for (int x = 0; x < Chunk::CHUNK_SIZE; x++) {
        for (int z = 0; z < Chunk::CHUNK_SIZE; z++) {

            float currentHeight = 0.0f;

            for (const TerrainLayer& layer : generationLayers) {
                if (!layer.isEnabled || !layer.algorithm) { continue; }

                if (layer.blendMode == BlendMode::CARVE) {

                    int absoluteBottom = std::clamp(layer.startY, 0, maxWorldHeight);
                    int absoluteTop = std::clamp(layer.endY, 0, maxWorldHeight);

                    for (int y = absoluteBottom; y <= absoluteTop; y++) {
                        float noise3DValue = layer.algorithm->evaluate3D(worldX + x, static_cast<float>(y), worldZ + z);

                        for (const auto& modification : layer.activeModifiers) {
                            noise3DValue = modification->modify(noise3DValue);
                        }

                        if (noise3DValue > layer.blendWeight) {
                            column.setBlock(x, y, z, layer.blockID);
                        }
                    }

                    continue;
                }

                float value = layer.algorithm->evaluate(worldX + x, worldZ + z);

                for (const auto& modification : layer.activeModifiers) {
                    value = modification->modify(value);
                }

                float mappedHeight = layer.startY + (value * (layer.endY - layer.startY));

                float newHeight = currentHeight;
                switch (layer.blendMode) {
                    case BlendMode::NORMAL:newHeight = mappedHeight; 
                        break;
                    case BlendMode::ADD:newHeight = currentHeight + mappedHeight;
                        break;
                    case BlendMode::SUBTRACT:newHeight = currentHeight - mappedHeight;
                        break;
                    case BlendMode::MULTIPLY:newHeight = currentHeight * value;
                        break;
                    case BlendMode::MAX:newHeight = std::max(currentHeight, mappedHeight);
                        break;
                    case BlendMode::MIN:newHeight = std::min(currentHeight, mappedHeight);
                        break;
                    case BlendMode::SMOOTH:newHeight = currentHeight + layer.blendWeight * (mappedHeight - currentHeight);
                        break;
                    case BlendMode::ABSOLUTE:newHeight = std::max(currentHeight, mappedHeight);
                        break;
                }

                int iCurrent = std::clamp(static_cast<int>(currentHeight), 0, maxWorldHeight);
                int iNew = std::clamp(static_cast<int>(newHeight), 0, maxWorldHeight);

                if (layer.blendMode == BlendMode::ABSOLUTE) {
                    int absoluteBottom = std::clamp(layer.startY, 0, 255);
                    int absoluteTop = std::clamp(static_cast<int>(mappedHeight), 0, 255);

                    for (int y = absoluteBottom; y <= absoluteTop; y++) {
                        column.setBlock(x, y, z, layer.blockID);
                    }
                }
                else {
                    if (iNew > iCurrent) {
                        for (int y = iCurrent; y < iNew; y++) {
                            column.setBlock(x, y, z, layer.blockID);
                        }
                    }
                    else if (iNew < iCurrent) {
                        for (int y = iNew; y < iCurrent; y++) {
                            column.setBlock(x, y, z, 0);
                        }
                    }
                }

                currentHeight = newHeight;
            }

        }
    }
}

void WorldTerrainGenerator::clearLayers() {
    generationLayers.clear();
}