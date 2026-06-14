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
	pendingOpaqueVertices.clear();
	pendingTransparentVertices.clear();
	pendingOpaqueIndices.clear();
	pendingTransparentIndices.clear();
	uint32_t opaqueIndexOffset = 0;
	uint32_t transparentIndexOffset	 = 0;


	pendingOpaqueVertices.reserve((Chunk::CHUNK_VOLUME * chunks.size()) / 2);
	pendingOpaqueIndices.reserve(Chunk::CHUNK_VOLUME * chunks.size());

	pendingTransparentVertices.reserve((Chunk::CHUNK_VOLUME * chunks.size()) / 4);
	pendingTransparentIndices.reserve((Chunk::CHUNK_VOLUME * chunks.size()) / 2);


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

		chunks[i]->collectMeshData(pendingOpaqueVertices, pendingOpaqueIndices, opaqueIndexOffset, pendingTransparentVertices, pendingTransparentIndices, transparentIndexOffset, chunkYOffset, topNeighbor, bottomNeighbor, frontNeighbor, backNeighbor, leftNeighbor, rightNeighbor);
	}

	hasPendingMeshData = true;
}



void ChunkColumn::uploadMeshToGPU()
{
	std::lock_guard<std::mutex> meshLock(meshMutex);

	if (!hasPendingMeshData) { return; }

	if (!pendingOpaqueVertices.empty()) {

		if (!opaqueColumnMesh) {
			opaqueColumnMesh = std::make_unique<Mesh>(pendingOpaqueVertices, pendingOpaqueIndices);
		}
		else 
		{
			opaqueColumnMesh->updateData(pendingOpaqueVertices, pendingOpaqueIndices);
		}

	}
	else if (opaqueColumnMesh) {
		opaqueColumnMesh->updateData(pendingOpaqueVertices, pendingOpaqueIndices);
	}


	if (!pendingTransparentVertices.empty()) {

		if (!transparentColumnMesh) {
			transparentColumnMesh = std::make_unique<Mesh>(pendingTransparentVertices, pendingTransparentIndices);
		}
		else
		{
			transparentColumnMesh->updateData(pendingTransparentVertices, pendingTransparentIndices);
		}

	}
	else if (transparentColumnMesh) {
		transparentColumnMesh->updateData(pendingTransparentVertices, pendingTransparentIndices);
	}

	pendingOpaqueVertices.clear();
	pendingOpaqueIndices.clear();

	pendingTransparentVertices.clear();
	pendingTransparentIndices.clear();

	hasPendingMeshData = false;
	isMeshGenerated = true;
	isRerenderNeeded = false;
}



void ChunkColumn::renderOpaque(Shader* shader) const
{
	if (!opaqueColumnMesh) { return; }

	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(columnX * Chunk::CHUNK_SIZE, 0, columnZ * Chunk::CHUNK_SIZE));
	shader->setMatrix4("model", model);

	opaqueColumnMesh->draw();
}



void ChunkColumn::renderTransparent(Shader* shader) const
{
	if (!transparentColumnMesh) { return; }

	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(columnX * Chunk::CHUNK_SIZE, 0, columnZ * Chunk::CHUNK_SIZE));
	shader->setMatrix4("model", model);

	transparentColumnMesh->draw();
}



int ChunkColumn::getX() const 
{ 
	return columnX; 
}



int ChunkColumn::getZ() const 
{ 
	return columnZ; 
}



std::mutex& ChunkColumn::getMeshMutex() 
{
	return meshMutex;
}



bool ChunkColumn::hasMesh() const 
{
	return isMeshGenerated;
}



bool ChunkColumn::needsRerender() const 
{
	return isRerenderNeeded;
}