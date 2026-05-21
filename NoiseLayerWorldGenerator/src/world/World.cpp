#include "world/World.h"

World::World() {

}

World::~World()
{
	isGenerating = false;

	if (generationThread.joinable()) {
		generationThread.join();
	}
}

WorldState World::getCurrentState() const {
	return currentState;
}

float World::getGenerationProgress() const {
	if (totalChunksToGenerate == 0) {
		return 0.0f;
	}

	return static_cast<float>(generatedChunksCount.load()) / static_cast<float>(totalChunksToGenerate);
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

	columnPtr->buildMeshFromPendingData(*this);
	columnPtr->uploadMeshToGPU();

	int xShift[] = { 1, -1, 0, 0 };
	int zShift[] = { 0, 0, 1, -1 };

	for (int i = 0; i < 4; i++) {
		ChunkColumn* neighbor = getChunkColumn(x + xShift[i], z + zShift[i]);
		if (neighbor) {
			neighbor->buildMeshFromPendingData(*this);
			neighbor->uploadMeshToGPU();
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

void World::setCamera(const Camera* camera)
{
	targetCamera = camera;
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
	for (auto const& it : columnsMap) {
		it.second->buildMeshFromPendingData(*this);
		it.second->uploadMeshToGPU();
	}
}

void World::render(Shader* shader) const
{
	for (auto const& it : columnsMap) {
		it.second->render(shader);
	}
}

void World::updateWorld()
{
	if (!uploadToGPUQueue.empty()) {
		std::lock_guard<std::mutex> lock(uploadQueueMutex);

		int uploadsThisFrame = 0;
		int maxUploadsPerFrame = 4; // Limitujemy ilość uploadów na klatkę

		while (!uploadToGPUQueue.empty() && uploadsThisFrame < maxUploadsPerFrame) {
			ChunkColumn* column = uploadToGPUQueue.back();
			uploadToGPUQueue.pop_back();
			column->uploadMeshToGPU();
			uploadsThisFrame++;
		}
	}

	if (currentState == WorldState::LOADING && generatedChunksCount == totalChunksToGenerate && uploadToGPUQueue.empty()) {
		currentState = WorldState::PLAYING;
		std::cout << "+[WORLD] W pelni wygenerowano swiat!" << std::endl;

		generationFutures.clear();
	}

	if (currentState == WorldState::PLAYING) {
		glm::vec3 cameraPosition = targetCamera->position;
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
	};
}

void World::clearWorld()
{
	isGenerating = false;
	if (generationThread.joinable()) {
		generationThread.join();
	}
	generationFutures.clear();

	columnsMap.clear();
	generatedChunksCount = 0;
	currentState = WorldState::LOADING;

	int side = (config.renderDistance * 2) + 1;
	totalChunksToGenerate = side * side;

	isGenerating = true;

	generationThread = std::thread([this]() {

		unsigned int availableThreads = std::thread::hardware_concurrency();
		if (availableThreads == 0) {
			availableThreads = 4;
		}

		unsigned int workerThreadsCount = 1;
		if (availableThreads > 1) {
			workerThreadsCount = availableThreads - 1;
		}

		glm::vec3 cameraPosition = glm::vec3(0.0f);
		if (targetCamera) {
			cameraPosition = targetCamera->position;
		}

		int cameraColumnX = static_cast<int>(std::floor(cameraPosition.x / (Chunk::CHUNK_SIZE)));
		int cameraColumnZ = static_cast<int>(std::floor(cameraPosition.z / (Chunk::CHUNK_SIZE)));

		std::mutex generatorMutex;

		for (unsigned int t = 0; t < workerThreadsCount; t++) {
			generationFutures.push_back(std::async(std::launch::async,
				[this, t, workerThreadsCount, cameraColumnX, cameraColumnZ, &generatorMutex]() {

					for (int x = -config.renderDistance + t; x <= config.renderDistance; x += workerThreadsCount) {
						for (int z = -config.renderDistance; z <= config.renderDistance; z++) {

							if (!isGenerating) { return; }

							ChunkCords coords = { cameraColumnX + x, cameraColumnZ + z };
							auto column = std::make_unique<ChunkColumn>(coords.x, coords.z);

							if (terrainGenerator) {
								std::lock_guard<std::mutex> genLock(generatorMutex);
								terrainGenerator->applyToColumn(*column);
							}

							{
								std::lock_guard<std::mutex> lock(uploadQueueMutex);
								columnsMap[coords] = std::move(column);
							}
						}

					}
				}));
		}

		for (auto& future : generationFutures) {
			if (future.valid()) {
				future.wait();
			}
		}
		generationFutures.clear();

		for (unsigned int t = 0; t < workerThreadsCount; t++) {
			generationFutures.push_back(std::async(std::launch::async,
				[this, t, workerThreadsCount, cameraColumnX, cameraColumnZ]() {

					for (int x = -config.renderDistance + t; x <= config.renderDistance; x += workerThreadsCount) {
						for (int z = -config.renderDistance; z <= config.renderDistance; z++) {

							if (!isGenerating) { return; }

							ChunkCords coords = { cameraColumnX + x, cameraColumnZ + z };
							ChunkColumn* colPtr = nullptr;

							{
								std::lock_guard<std::mutex> lock(uploadQueueMutex);
								colPtr = getChunkColumn(coords.x, coords.z);
							}

							if (colPtr) {
								colPtr->buildMeshFromPendingData(*this);

								{
									std::lock_guard<std::mutex> lock(uploadQueueMutex);
									uploadToGPUQueue.push_back(colPtr);
								}

								generatedChunksCount++;
							}
						}
					}

				}));
		}

		for (auto& future : generationFutures) {
			if (future.valid()) future.wait();
		}

		generationFutures.clear();
		});

	std::cout << "+[WORLD] Pomyslnie utworzono wszystkie watki" << std::endl;
}