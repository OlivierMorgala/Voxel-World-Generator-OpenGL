#pragma once
#include <glm/glm.hpp>
#include "world/Chunk.h"
#include "world/ChunkColumn.h"
#include "world/WorldTerrainGenerator.h"
#include "Camera.h"
#include <future>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>
#include <vector>
#include <map>

#include "WorldConfig.h"

enum class WorldState {
	PLAYING,
	LOADING
};

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
	WorldState currentState = WorldState::PLAYING;

    std::thread generationThread;
	std::vector<std::future<void>> generationFutures;
	std::atomic<bool> isGenerating = false;
	std::atomic<int> generatedChunksCount = 0;
	int totalChunksToGenerate = 0;
    
	std::vector<ChunkColumn*> uploadToGPUQueue;
	std::mutex uploadQueueMutex;

	const Camera* targetCamera = nullptr;

    WorldTerrainGenerator* terrainGenerator = nullptr;
    std::map<ChunkCords, std::unique_ptr<ChunkColumn>> columnsMap;

public:
    World();
    ~World();

    WorldState getCurrentState() const;
    float getGenerationProgress() const;

	void setCamera(const Camera* camera);

    void setTerrainGenerator(WorldTerrainGenerator* generator);

    void setBlock(int x, int y, int z, BlockID blockID);
    BlockID getBlock(int x, int y, int z) const;

    void addChunkColumn(int x, int z);
    ChunkColumn* getChunkColumn(int x, int z) const;

    const std::map<ChunkCords, std::unique_ptr<ChunkColumn>>& getColumnsMap() const;

    void generateWorldMesh();
    void render(Shader* shader) const;
    void updateWorld(); 
    void clearWorld(); 
};

