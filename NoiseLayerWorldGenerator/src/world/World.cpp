#include "world/World.h"

World::World() {
	unsigned int threadCount = std::thread::hardware_concurrency();

	if(threadCount == 0) {
		threadCount = 1;
	}

	unsigned int workerThreadCount = 1;
	if (threadCount > 1) {
		workerThreadCount = threadCount - 1;
	}

	for (unsigned int i = 0; i < workerThreadCount; i++) {
		workerThreads.emplace_back([this]() {
			while (true) {
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> tasksQueueLock(tasksQueueMutex);
					this->condition.wait(tasksQueueLock, [this]() { 
						return this->stopPool || !this->tasksQueue.empty();
					});

					if (this->stopPool && this->tasksQueue.empty()) {
						return;
					}

					task = std::move(this->tasksQueue.front());
					this->tasksQueue.pop();
				}
				task();
			}
		});
	}
}



World::~World()
{
	isGenerating = false;

	{
		std::unique_lock<std::mutex> tasksQueueLock(tasksQueueMutex);
		stopPool = true;
	}
	condition.notify_all();

	for (std::thread& worker : workerThreads) {
		if (worker.joinable()) {
			worker.join();
		}
	}

	if (generationThread.joinable()) {
		generationThread.join();
	}

	while (pendingTasks > 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}



void World::enqueueTask(std::function<void()> task)
{
	pendingTasks++;

	{
		std::lock_guard<std::mutex> tasksQueueLock(tasksQueueMutex);
		tasksQueue.push([this, task]() {
			task();
			pendingTasks--;
			});
	}

	condition.notify_one();
}



void World::setCamera(const Camera* camera)
{
	targetCamera = camera;
}




void World::setTerrainGenerator(WorldTerrainGenerator* generator) {
	terrainGenerator = generator;
}



WorldState World::getCurrentState() const {
	return currentState;
}



float World::getGenerationProgress() const {
	if (totalChunksToGenerate == 0) {
		return 0.0f;
	}

	float totalTasks = static_cast<float>(totalChunksToGenerate * 2);
	float completedTasks = static_cast<float>(generatedChunksCount.load() + uploadedChunksCount.load());

	return completedTasks / totalTasks;
}



void World::getLocalCoords(int globalX, int globalZ, int& columnX, int& columnZ, int& localX, int& localZ) const
{
	columnX = globalX >> 4;
	columnZ = globalZ >> 4;
	localX = globalX & 15;
	localZ = globalZ & 15;
}



void World::addChunkColumn(int x, int z)
{
	std::cout << "+[WORLD] Generuje kolumne: " << x << ", " << z << std::endl;

	ChunkCords position = { x, z };
	auto column = std::make_unique<ChunkColumn>(x, z);

	if (terrainGenerator) {
		terrainGenerator->applyToColumn(*column);
	}

	ChunkColumn* columnPtr = column.get();

	{
		std::unique_lock<std::shared_mutex> columnsMapLock(columnsMapMutex);
		columnsMap[position] = std::move(column);
	}

	{
		std::lock_guard<std::mutex> meshLock(columnPtr->getMeshMutex());
		columnPtr->buildMeshFromPendingData(*this);
	}

	constexpr int neighborShiftX[] = { 1, -1, 0, 0 };
	constexpr int neighborShiftZ[] = { 0, 0, 1, -1 };
	
	for (int i = 0; i < 4; i++) {
		ChunkColumn* neighborColumn = getChunkColumn(x + neighborShiftX[i], z + neighborShiftZ[i]);

		if (neighborColumn) {
			std::lock_guard<std::mutex> neighborMeshLock(neighborColumn->getMeshMutex());
			neighborColumn->buildMeshFromPendingData(*this);
			
			bool expected = false;
			if (neighborColumn->isMeshUploadPending.compare_exchange_strong(expected, true)) {
				std::lock_guard<std::mutex> uploadQueueLock(uploadQueueMutex);
				uploadToGPUQueue.push_back(neighborColumn);
			}
		}
	}


	
	bool expected = false;
	if (columnPtr->isMeshUploadPending.compare_exchange_strong(expected, true)) {
		std::lock_guard<std::mutex> uploadQueueLock(uploadQueueMutex);
		uploadToGPUQueue.push_back(columnPtr);
	}
	
}



ChunkColumn* World::getChunkColumn(int x, int z) const
{
	std::shared_lock<std::shared_mutex> columnsMapLock(columnsMapMutex);

	auto it = columnsMap.find({x, z});

	if (it != columnsMap.end()) {
		return it->second.get();
	}

	return nullptr;
}



void World::setBlock(int x, int y, int z, BlockID blockID)
{

	if (y < 0 || y >= config.worldHeightInChunks * Chunk::CHUNK_SIZE) 
	{
		return;
	}

	int columnX;
	int columnZ;
	int localX;
	int localZ;

	getLocalCoords(x, z, columnX, columnZ, localX, localZ);

	if (auto column = getChunkColumn(columnX, columnZ))
	{
		column->setBlock(localX, y, localZ, blockID);
	}
}

void World::renderAlteredChunks(ChunkColumn* column) // METODA ODPOWIADA ZA PONOWNE RENDEROWANIE CHUNKOW KTORE ZOSTALY ZMIENIONE PRZEZ UZYTKOWNIKA -> CZYLI TAKIE NA KTORYM ZNISZCZONO LUB POSTAWIONO BLOK
{
	enqueueTask([this, column]() {
		{
			std::lock_guard<std::mutex> meshLock(column->getMeshMutex());
			column->buildMeshFromPendingData(*this);
		}

		{
			std::lock_guard<std::mutex> lock(uploadQueueMutex);
			uploadToGPUQueue.push_back(column);
		}

		});
}



BlockID World::getBlock(int x, int y, int z) const
{
	if (y < 0 || y >= config.worldHeightInChunks * Chunk::CHUNK_SIZE) {
		return 0;
	}

	int columnX;
	int columnZ;
	int localX;
	int localZ;

	getLocalCoords(x, z, columnX, columnZ, localX, localZ);

	if (auto column = getChunkColumn(columnX, columnZ)) {
		return column->getBlock(localX, y, localZ);
	}

	return 0;
}



void World::generateWorldMesh()
{
	std::shared_lock<std::shared_mutex> columnsMapLock(columnsMapMutex);

	for (auto const& [position, column] : columnsMap) {
		column->buildMeshFromPendingData(*this);
		column->uploadMeshToGPU();
	}
}



void World::render(Shader* shader) const
{
	std::shared_lock<std::shared_mutex> columnsMapLock(columnsMapMutex);
	for (auto const& [position, column] : columnsMap) {
		column->render(shader);
	}
}



void World::updateWorld()
{
	std::vector<ChunkColumn*> chunksToUploadThisFrame;

	{
		std::lock_guard<std::mutex> uploadQueueLock(uploadQueueMutex);
		int chunksProcessedThisFrame = 0;

		while (!uploadToGPUQueue.empty() && chunksProcessedThisFrame < MAX_CHUNKS_UPLOADED_PER_FRAME) {
			ChunkColumn* column = uploadToGPUQueue.front();
			uploadToGPUQueue.pop_front();
			column->isMeshUploadPending = false;

			chunksToUploadThisFrame.push_back(column);
			chunksProcessedThisFrame++;
			uploadedChunksCount++;
		}
	}

	for (ChunkColumn* column : chunksToUploadThisFrame) {
		column->uploadMeshToGPU();
	}

	if (currentState == WorldState::LOADING && uploadedChunksCount >= totalChunksToGenerate) {
		currentState = WorldState::PLAYING;
		std::cout << "+[WORLD] Zakonczono generowanie swiata!" << std::endl;
	}

	if (currentState == WorldState::PLAYING) {

		if(!targetCamera) { return; }
		int cameraColumnX = static_cast<int>(std::floor(targetCamera->position.x / Chunk::CHUNK_SIZE));
		int cameraColumnZ = static_cast<int>(std::floor(targetCamera->position.z / Chunk::CHUNK_SIZE));

		int chunksQueuedThisFrame = 0;
		std::vector<ChunkCords> chunksToGenerateThisFrame;

		{
			std::shared_lock<std::shared_mutex> columnsMapLock(columnsMapMutex);
			std::lock_guard<std::mutex> generationSetLock(generationSetMutex);

			for (int d = 0; d <= config.renderDistance; d++) {
				for(int x = -d; x <= d; x++) {
					for(int z = -d; z <= d; z++) {

						if(std::abs(x) != d && std::abs(z) != d) {
							continue;
						}

						ChunkCords coords = { cameraColumnX + x, cameraColumnZ + z };

						if (columnsMap.find(coords) != columnsMap.end()) {
							continue;
						}

						if (columnsCurrentlyGenerating.find(coords) == columnsCurrentlyGenerating.end()) {
							chunksToGenerateThisFrame.push_back(coords);
							columnsCurrentlyGenerating.insert(coords);
							chunksQueuedThisFrame++;

							if (chunksQueuedThisFrame >= MAX_CHUNKS_GENERATED_PER_FRAME) {
								break;
							}
						}

					}
					if (chunksQueuedThisFrame >= MAX_CHUNKS_GENERATED_PER_FRAME) { break; }
				}
				if (chunksQueuedThisFrame >= MAX_CHUNKS_GENERATED_PER_FRAME) { break; }
			}
		}

		for (const auto& coords : chunksToGenerateThisFrame) {
			std::cout << "+[WORLD] Dodano kolumne do generowania: " << coords.x << ", " << coords.z << std::endl;

			
			enqueueTask([this, coords]() {
				if(!isGenerating) { return; }
				addChunkColumn(coords.x, coords.z);

				std::lock_guard<std::mutex> generationSetLock(generationSetMutex);
				columnsCurrentlyGenerating.erase(coords);
				});
		}




		//UNLOAD




	}
}



void World::regenerateWorld()
{
	isGenerating = false;
	if (generationThread.joinable()) {
		generationThread.join();
	}

	while (pendingTasks > 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}


	{
		std::unique_lock<std::shared_mutex> columnsMapLock(columnsMapMutex);
		std::lock_guard<std::mutex> uploadQueueLock(uploadQueueMutex);
		std::lock_guard<std::mutex> generationSetLock(generationSetMutex);
		columnsMap.clear();
		uploadToGPUQueue.clear();
		columnsCurrentlyGenerating.clear();
	}

	generatedChunksCount = 0;
	uploadedChunksCount = 0;
	currentState = WorldState::LOADING;
	isGenerating = true;

	generationThread = std::thread([this]() {

		std::vector<std::future<void>> localFutures;

		unsigned int availableThreads = std::thread::hardware_concurrency();
		if (availableThreads == 0) {
			availableThreads = 4;
		}

		unsigned int workerThreadsCount = 1;
		if (availableThreads > 1) {
			workerThreadsCount = availableThreads - 1;
		}

		for (unsigned int t = 0; t < workerThreadsCount; t++) {
			enqueueTask([this, t, workerThreadsCount]() {

					for (int x = -config.renderDistance + t; x <= config.renderDistance; x += workerThreadsCount) {
						for (int z = -config.renderDistance; z <= config.renderDistance; z++) {

							if (!isGenerating) { return; }

							ChunkCords coords = {x, z};
							auto column = std::make_unique<ChunkColumn>(coords.x, coords.z);

							if (terrainGenerator) {
								terrainGenerator->applyToColumn(*column);
							}

							{
								std::unique_lock<std::shared_mutex> columnsMapLock(columnsMapMutex);
								columnsMap[coords] = std::move(column);
							}
						}

					}
				});
		}

		while (pendingTasks > 0) {
			if (!isGenerating) {
				return;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		for (unsigned int t = 0; t < workerThreadsCount; t++) {
			enqueueTask([this, t, workerThreadsCount]() {

					for (int x = -config.renderDistance + t; x <= config.renderDistance; x += workerThreadsCount) {
						for (int z = -config.renderDistance; z <= config.renderDistance; z++) {

							if (!isGenerating) { return; }

							ChunkCords coords = { x, z};
							ChunkColumn* colPtr = getChunkColumn(coords.x, coords.z);

							if (colPtr) {
								{
									std::lock_guard<std::mutex> meshLock(colPtr->getMeshMutex());
									colPtr->buildMeshFromPendingData(*this);
								}

								bool expected = false;
								if(colPtr->isMeshUploadPending.compare_exchange_strong(expected, true)) {
									std::lock_guard<std::mutex> lock(uploadQueueMutex);
									uploadToGPUQueue.push_back(colPtr);
								}

								generatedChunksCount++;
							}
						}
					}

				});
		}
	});

	std::cout << "+[WORLD] Pomyslnie utworzono wszystkie watki" << std::endl;
}