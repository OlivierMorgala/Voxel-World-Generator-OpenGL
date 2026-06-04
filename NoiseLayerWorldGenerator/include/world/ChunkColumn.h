#pragma once
#include "Mesh.h"
#include "Shader.h" 
#include "BlockType.h"
#include <vector>
#include <memory>
#include <cstdint>

#include <atomic>
#include <mutex>

class World;
class Chunk;

class ChunkColumn
{
private:
	int columnX;
	int columnZ;

	std::unique_ptr<Mesh> columnMesh;

	bool isMeshGenerated = false;
	bool isRerenderNeeded = false;
	bool hasPendingMeshData = false;

	std::vector<Vertex> pendingVertices;
	std::vector<uint32_t> pendingIndicies;

	std::mutex meshMutex;

	std::vector<std::unique_ptr<Chunk>> chunks;

public:
	ChunkColumn(int x, int z);
	~ChunkColumn() = default;

	std::atomic<bool> isMeshUploadPending = false;

	void setBlock(int x, int y, int z, BlockID blockID);
	BlockID getBlock(int x, int y, int z) const;

	Chunk* getChunk(int yIndex) const;

	void buildMeshFromPendingData(const World& world);
	void uploadMeshToGPU();
	void render(Shader* shader) const;

	int getX() const;
	int getZ() const;

	std::mutex& getMeshMutex();

	bool hasMesh() const;
	bool needsRerender() const;
};

