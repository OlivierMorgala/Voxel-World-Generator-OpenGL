#pragma once
#include <glm/glm.hpp>
#include "world/Chunk.h"
#include "world/ChunkColumn.h"
#include "world/WorldTerrainGenerator.h"
#include "Camera.h"

#include <future>
#include <thread>
#include <shared_mutex>
#include <mutex>
#include <atomic>

#include <vector>
#include <queue>
#include <deque>
#include <map>
#include <unordered_set>
#include <algorithm>

#include "WorldConfig.h"

constexpr int MAX_CHUNKS_GENERATED_PER_FRAME = 1;
constexpr int MAX_CHUNKS_UPLOADED_PER_FRAME = 6;

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

    bool operator==(const ChunkCords& other) const {
        return x == other.x && z == other.z;
    }
};

struct ChunkCordsHash {
    std::size_t operator()(const ChunkCords& cords) const {
        return std::hash<int>()(cords.x) ^ (std::hash<int>()(cords.z) << 1);
    }
};

class World
{
private:
	WorldState currentState = WorldState::PLAYING;

	std::vector<std::thread> workerThreads;
	std::queue<std::function<void()>> tasksQueue;
	std::mutex tasksQueueMutex;
    std::condition_variable condition;
    std::atomic<bool> stopPool;

    std::thread generationThread;
    std::atomic<int> pendingTasks = 0;

	std::atomic<bool> isGenerating = false;
	std::atomic<int> generatedChunksCount = 0;
	std::atomic<int> uploadedChunksCount = 0;
	int totalChunksToGenerate = (2 * config.renderDistance + 1) * (2 * config.renderDistance + 1);
    
	std::deque<ChunkColumn*> uploadToGPUQueue;
	mutable std::mutex uploadQueueMutex;

    std::unordered_set<ChunkCords, ChunkCordsHash> columnsCurrentlyGenerating;
	std::mutex generationSetMutex;

	const Camera* targetCamera = nullptr;

    WorldTerrainGenerator* terrainGenerator = nullptr;
    std::unordered_map<ChunkCords, std::unique_ptr<ChunkColumn>, ChunkCordsHash> columnsMap;
    mutable std::shared_mutex columnsMapMutex;

    void enqueueTask(std::function<void()> task);

	void getLocalCoords(int globalX, int globalZ, int& columnX, int& columnZ, int& localX, int& localZ) const;

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
    std::vector<ChunkColumn*> getLoadedColumns() const;

    void generateWorldMesh();
    void updateWorld(); 
    void regenerateWorld(); 

    int getLoadedChunkColumnsCount();
};

