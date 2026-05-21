#include "world/ChunkColumn.h"
#include "world/World.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "world/WorldConfig.h"


ChunkColumn::ChunkColumn(int x, int z) : columnX(x), columnZ(z)
{
	chunks.reserve(config.worldHeightInChunks);
	for (int i = 0; i < config.worldHeightInChunks; i++) {
		chunks.push_back(std::make_unique<Chunk>());
	}
}

void ChunkColumn::setBlock(int x, int y, int z, BlockID blockID)
{
	if (y < 0 || y >= chunks.size() * Chunk::CHUNK_SIZE) { return; }
	if (x < 0 || x >= Chunk::CHUNK_SIZE || z < 0 || z >= Chunk::CHUNK_SIZE) { return; }

	int chunkIndex = y / Chunk::CHUNK_SIZE;
	int localY = y % Chunk::CHUNK_SIZE;

	chunks[chunkIndex]->setBlock(x, localY, z, blockID);

	isRerenderNeeded = true;
}

BlockID ChunkColumn::getBlock(int x, int y, int z) const
{
	if (y < 0 || y >= chunks.size() * Chunk::CHUNK_SIZE) { return 0; }
	if (x < 0 || x >= Chunk::CHUNK_SIZE || z < 0 || z >= Chunk::CHUNK_SIZE) { return 0; }

	int chunkIndex = y / Chunk::CHUNK_SIZE;
	int localY = y % Chunk::CHUNK_SIZE;

	return chunks[chunkIndex]->getBlock(x, localY, z);
}

Chunk* ChunkColumn::getChunk(int yIndex) const 
{
	if (yIndex >= 0 && yIndex < chunks.size()) {
		return chunks[yIndex].get();
	}
	return nullptr;
}

void ChunkColumn::buildMeshFromPendingData(const World& world)
{
	pendingVertices.clear();
	pendingIndicies.clear();
	uint32_t indexOffset = 0;

	pendingVertices.reserve(Chunk::CHUNK_VOLUME * 2);

	ChunkColumn* frontColumn = world.getChunkColumn(columnX, columnZ + 1);
	ChunkColumn* backColumn = world.getChunkColumn(columnX, columnZ - 1);
	ChunkColumn* leftColumn = world.getChunkColumn(columnX - 1, columnZ);
	ChunkColumn* rightColumn = world.getChunkColumn(columnX + 1, columnZ);

	for (int i = 0; i < chunks.size(); i++) {

		Chunk* topNeighbor = nullptr;
		if (i < chunks.size() - 1) {
			topNeighbor = chunks[i + 1].get();
		}

		Chunk* bottomNeighbor = nullptr;
		if (i > 0) {
			bottomNeighbor = chunks[i - 1].get();
		}

		Chunk* frontNeighbor = nullptr;
		if (frontColumn) {
			frontNeighbor = frontColumn->getChunk(i);
		}

		Chunk* backNeighbor = nullptr;
		if (backColumn) {
			backNeighbor = backColumn->getChunk(i);
		}

		Chunk* leftNeighbor = nullptr;
		if (leftColumn) {
			leftNeighbor = leftColumn->getChunk(i);
		}

		Chunk* rightNeighbor = nullptr;
		if (rightColumn) {
			rightNeighbor = rightColumn->getChunk(i);
		}

		int chunkYOffset = i * Chunk::CHUNK_SIZE;

		chunks[i]->collectMeshData(pendingVertices, pendingIndicies, indexOffset, chunkYOffset, topNeighbor, bottomNeighbor, frontNeighbor, backNeighbor, leftNeighbor, rightNeighbor);
	}

	hasPendingMeshData = true;
}

void ChunkColumn::uploadMeshToGPU()
{
	if (!hasPendingMeshData) { return; }
	
	if (!pendingVertices.empty()) {
		columnMesh = std::make_unique<Mesh>(pendingVertices, pendingIndicies);
	}
	else {
		columnMesh.reset();
	}

	pendingVertices.clear();
	pendingVertices.shrink_to_fit();
	pendingIndicies.clear();
	pendingIndicies.shrink_to_fit();

	hasPendingMeshData = false;
	isMeshGenerated = true;
	isRerenderNeeded = false;
}

void ChunkColumn::render(Shader* shader) const
{
	if (!columnMesh) { return; }

	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(columnX * Chunk::CHUNK_SIZE, 0, columnZ * Chunk::CHUNK_SIZE));
	shader->setMatrix4("model", model);

	columnMesh->draw();
}

int ChunkColumn::getX() const 
{ 
	return columnX; 
}

int ChunkColumn::getZ() const 
{ 
	return columnZ; 
}

bool ChunkColumn::hasMesh() const 
{
	return isMeshGenerated;
}

bool ChunkColumn::needsRerender() const 
{
	return isRerenderNeeded;
}