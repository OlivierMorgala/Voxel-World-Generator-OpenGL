#include "world/World.h"

World::World() {

}

void World::addChunkColumn(int x, int z) 
{
	ChunkCords position = { x, z };

	std::cout << "+[WORLD] Generuje kolumne: " << x << ", " << z << std::endl;
	auto column = std::make_unique<ChunkColumn>(x, z);

	if (terrainGenerator) {
		terrainGenerator->applyToColumn(*column);
	}

	ChunkColumn* columnPtr = column.get();
	columnsMap[position] = std::move(column);

	columnPtr->generateMeshes(*this);

	int xShift[] = { 1, -1, 0, 0 };
	int zShift[] = { 0, 0, 1, -1 };

	for (int i = 0; i < 4; i++) {
		ChunkColumn* neighbor = getChunkColumn(x + xShift[i], z + zShift[i]);
		if (neighbor) {
			neighbor->generateMeshes(*this);
		}
	}
}

ChunkColumn* World::getChunkColumn(int x, int z) const 
{
	ChunkCords position = { x, z };
	auto it = columnsMap.find(position);

	if (it != columnsMap.end()) {
		return it->second.get();
	}

	return nullptr;
}

const std::map<ChunkCords, std::unique_ptr<ChunkColumn>>& World::getColumnsMap() const 
{
	return columnsMap;
}

void World::setTerrainGenerator(WorldTerrainGenerator* generator) {
	terrainGenerator = generator;
}

void World::setBlock(int x, int y, int z, BlockID blockID)
{

	if (y < 0 || y >= config.worldHeightInChunks * Chunk::CHUNK_SIZE) {
		return;
	}

	int columnX = x / Chunk::CHUNK_SIZE;
	int columnZ = z / Chunk::CHUNK_SIZE;

	ChunkColumn* column = getChunkColumn(columnX, columnZ);

	if (column) {
		int localX = x % Chunk::CHUNK_SIZE;
		int localZ = z % Chunk::CHUNK_SIZE;

		if (localX < 0) { localX += Chunk::CHUNK_SIZE; }
		if (localZ < 0) { localZ += Chunk::CHUNK_SIZE; }

		column->setBlock(localX, y, localZ, blockID);
	}
}

BlockID World::getBlock(int x, int y, int z) const 
{
	if (y < 0 || y >= config.worldHeightInChunks * Chunk::CHUNK_SIZE) {
		return 0;
	}
	
	int columnX = x / Chunk::CHUNK_SIZE;
	int columnZ = z / Chunk::CHUNK_SIZE;

	ChunkColumn* column = getChunkColumn(columnX, columnZ);

	if (column) {
		int localX = x % Chunk::CHUNK_SIZE;
		int localZ = z % Chunk::CHUNK_SIZE;

		if (localX < 0) { localX += Chunk::CHUNK_SIZE; }
		if (localZ < 0) { localZ += Chunk::CHUNK_SIZE; }

		return column->getBlock(localX, y, localZ);
	}

	return 0;
}

void World::generateWorldMesh() 
{
	for(auto const& it : columnsMap) {
		it.second->generateMeshes(*this);
	}	
}

void World::render(Shader* shader) const 
{
	for (auto const& it : columnsMap) {
		it.second->render(shader);
	}
}

void World::updateWorld(glm::vec3 cameraPosition)
{
	int cameraColumnX = static_cast<int>(std::floor(cameraPosition.x / (Chunk::CHUNK_SIZE)));
	int cameraColumnZ = static_cast<int>(std::floor(cameraPosition.z / (Chunk::CHUNK_SIZE)));

	for (int x = -config.renderDistance; x <= config.renderDistance; x++) {
		for (int z = -config.renderDistance; z <= config.renderDistance; z++) {

			ChunkCords coords = { cameraColumnX + x, cameraColumnZ + z };

			if (columnsMap.find(coords) == columnsMap.end()) {
				addChunkColumn(coords.x, coords.z);
			}
		}
	}
}

void World::clearWorld() 
{
	columnsMap.clear();
	std::cout << "-[WORLD] Oczyszczono swiat z wszystkich chunkow" << std::endl;
}