#pragma once
#include <glm/glm.hpp>
#include "world/Chunk.h"
#include "world/ChunkColumn.h"
#include "world/WorldTerrainGenerator.h"
#include <vector>
#include <map>

#include "WorldConfig.h"

struct ChunkCords {
    int x, z;

    // Musimy przeciążyć operator <, aby std::map wiedziała jak sortować klucze (wartości po których szuka)
    bool operator<(const ChunkCords& other) const {
        if (x != other.x) { return x < other.x; }
        return z < other.z;
    }
};

class World
{
private:
    WorldTerrainGenerator* terrainGenerator = nullptr;

    std::map<ChunkCords, std::unique_ptr<ChunkColumn>> columnsMap;

public:
    World();
    ~World() = default;

    void setTerrainGenerator(WorldTerrainGenerator* generator);

    void setBlock(int x, int y, int z, BlockID blockID);
    BlockID getBlock(int x, int y, int z) const;

    void addChunkColumn(int x, int z);
    ChunkColumn* getChunkColumn(int x, int z) const;

    const std::map<ChunkCords, std::unique_ptr<ChunkColumn>>& getColumnsMap() const;

    void generateWorldMesh();
    void render(Shader* shader) const;
    void updateWorld(glm::vec3 cameraPosition);
    void clearWorld();
};

