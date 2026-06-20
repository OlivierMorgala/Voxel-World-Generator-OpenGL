#include "world/World.h"

World::World() {
	//Pobiertamy liczbe dostępnych rdzemi procesora
	unsigned int threadCount = std::thread::hardware_concurrency();
	if(threadCount == 0) {
		threadCount = 1;
	}

	//Utalamy liczbe wątków robotników (jeden zostawiamy dla głównego wątku)
	unsigned int workerThreadCount = 1;
	if (threadCount > 1) {
		workerThreadCount = threadCount - 1;
	}


	//Twworzymy pule wątków. Każdy wątek opczekuje na zadanie w nieskończonej pętli
	for (unsigned int i = 0; i < workerThreadCount; i++) {
		workerThreads.emplace_back([this]() {
			while (true) {
				std::function<void()> task;

				{
					//Czekamy aż zwolni się mutex i pojawi się zadanie w kolejce
					std::unique_lock<std::mutex> tasksQueueLock(tasksQueueMutex);
					this->condition.wait(tasksQueueLock, [this]() { 
						return this->stopPool || !this->tasksQueue.empty();
					});

					//Jeśli dostaliśmy polecenie zatrzymania i kolejka jest pusta kończymy działanie
					if (this->stopPool && this->tasksQueue.empty()) {
						return;
					}

					//Pobeieranie zadania z przodu kolejki i usuniecie go 
					task = std::move(this->tasksQueue.front());
					this->tasksQueue.pop();
				}

				//Wykonujemy zadanie (poza mutexem by inne wątki nie musiały czekać)
				task();
			}
		});
	}
}



World::~World()
{
	//Zamykamy bezpiecznie wątki zmieniając flage generacji i potem zmaienaimy flage stop  na true
	isGenerating = false;

	{
		std::unique_lock<std::mutex> tasksQueueLock(tasksQueueMutex);
		stopPool = true;
	}
	condition.notify_all();

	//czekamy na zakończenie pracy każdego wątku roboczego
	for (std::thread& worker : workerThreads) {
		if (worker.joinable()) {
			worker.join();
		}
	}

	//informujemy również wątek generacji początkowej
	if (generationThread.joinable()) {
		generationThread.join();
	}

	//upewniamy się że napewno wszystkie zadania dobiegły końca
	while (pendingTasks > 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}



void World::enqueueTask(std::function<void()> task)
{
	//Zwiększamy licznik aktywnych zadań
	pendingTasks++;

	{
		std::lock_guard<std::mutex> tasksQueueLock(tasksQueueMutex);

		tasksQueue.push([this, task]() {
			//Próbujemy dodać zadanie do koleki
			try {
				task();
			}catch (const std::exception& e) {
				std::cerr << "[ERROR:WORLD::THREAD] Zadanie przerwane: " << e.what() << std::endl;
			}
			pendingTasks--;
		});
	}

	//wybudzamy jeden wątek by zaczął wykonywać zadanie
	condition.notify_one();
}


//Przypisujemy obiekt kamery do obeiktu świata
void World::setCamera(const Camera* camera)
{
	targetCamera = camera;
}


//Przypisujemy obiekt generatora terenu do obeiktu świata
void World::setTerrainGenerator(WorldTerrainGenerator* generator) {
	terrainGenerator = generator;
}


//Zwaracamy obecny stan świata generacja/aktywny
WorldState World::getCurrentState() const {
	return currentState;
}


//Metoda obliczająca postęp gheneracji świata dla sceny LoadinScene na podstawie zmiennych atomowych
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
	//Obliczamy współrzędne przy użyciu przesunięć bitowych dla lepszej wydajnosci.

	//Uzyskujemy globalne współrzędne kolumny (>>4 dzielenie przez 16)
	columnX = globalX >> 4;
	columnZ = globalZ >> 4;

	//Uzyskujemy lokalne współrzędne wewnątrz chunka (&15 modulo 16)
	localX = globalX & 15;
	localZ = globalZ & 15;
}


//Metoda zwaraca ilość załadowanych kolumn (potrzebne do wyświetlania HUD z iloscią chunków)
int World::getLoadedChunkColumnsCount()
{
	std::shared_lock<std::shared_mutex> columnsMapLock(columnsMapMutex);
	return columnsMap.size();
}



void World::addChunkColumn(int x, int z)
{
	std::cout << "+[WORLD] Generuje kolumne: " << x << ", " << z << std::endl;

	//Tworzymy nową kolumne w pamięci
	ChunkCords position = { x, z };
	auto column = std::make_shared<ChunkColumn>(x, z);

	//Używamy generatora terenu do obliczenia szumu dla kolumny (zbioru szumów i modyfikacji)
	if (terrainGenerator) {
		terrainGenerator->applyToColumn(*column);
	}

	std::shared_ptr<ChunkColumn> columnPtr = column;

	{
		//Zapisujemy gotową kolumnę do mapy świata 
		std::unique_lock<std::shared_mutex> columnsMapLock(columnsMapMutex);
		columnsMap[position] = column;
	}

	//Dodajemy do kolejki zadanie budowy nowej siatki dla kolumny
	enqueueTask([this, columnPtr]() {
		{
			//Blokujemy mutex siatki w kolumnie aby uniknąć błędów z budowaniem danych vertexów
			std::lock_guard<std::mutex> meshLock(columnPtr->getMeshMutex());
			columnPtr->buildMeshFromPendingData(*this);
		}

		//Jeśli flaga oczekiwania na upload jest zaznaczona to dodajemy kolumnę do kolejki GPU
		bool expected = false;
		if (columnPtr->isMeshUploadPending.compare_exchange_strong(expected, true)) {
			std::lock_guard<std::mutex> uploadQueueLock(uploadQueueMutex);
			uploadToGPUQueue.push_back(columnPtr);
		}
	});

	//Aktualizacaj siatki 4 sąsiadów dookoła ponieważ wygenerowanie nowej kolumny mogło o odsłonić/zasłonić scianki graniczące między chunkami
	constexpr int neighborShiftX[] = { 1, -1, 0, 0 };
	constexpr int neighborShiftZ[] = { 0, 0, 1, -1 };
	
	for (int i = 0; i < 4; i++) {
		//Pobieramy wskaźnik na jednaego z sądsaidów 
		std::shared_ptr<ChunkColumn> neighborColumn = getChunkColumn(x + neighborShiftX[i], z + neighborShiftZ[i]);

		//Jeśli sąsiad istnieje w pamięci zlecamy przebudowe jego siatki
		if (neighborColumn) {
			enqueueTask([this, neighborColumn]() {
				{
					//Zabezpieczamy dane wierzchołków sąsiada na czas ich przeliczania
					std::lock_guard<std::mutex> neighborMeshLock(neighborColumn->getMeshMutex());
					neighborColumn->buildMeshFromPendingData(*this);
				}

				//Upeniamy się że sąsiad trafił do kolejki uploadToGpuQueue tylko raz. Jeśli kilka wątków jednocześnie zaktualizowało sąsiada z różnych stron
				//tylko pierwszy wątek zmieni flage na true i wrzuci go do kolejki
				bool expected = false;
				if (neighborColumn->isMeshUploadPending.compare_exchange_strong(expected, true)) {
					std::lock_guard<std::mutex> uploadQueueLock(uploadQueueMutex);
					uploadToGPUQueue.push_back(neighborColumn);
				}
			});
		}
	}
	
}


//Metoda pobierająca kolumne o kordynatach x oraz z w siatce kolumn
std::shared_ptr<ChunkColumn> World::getChunkColumn(int x, int z) const
{
	//Zakładmy blokade dzieloną która umożliiwia wieu wątkom na raz pobierać dane z mapy
	std::shared_lock<std::shared_mutex> columnsMapLock(columnsMapMutex);

	//Przeszukujemny mape
	auto it = columnsMap.find({x, z});

	//Jeśli kolumna istnieje zwracamy na nią wskaźnik
	if (it != columnsMap.end()) {
		return it->second;
	}

	return nullptr;
}



std::vector<ChunkColumn*> World::getLoadedColumns() const 
{
	//Zakładmy blokade dzieloną która umożliiwia wieu wątkom na raz pobierać dane z mapy
	std::shared_lock<std::shared_mutex> columnsMapLock(columnsMapMutex);

	//Z góry rezerwujemy miejsce w wektorze ponieważ wiemy ile jest elementów w mapie (unikamy ponowych alokacji pamieci przy powiekszaniu wektora)
	std::vector<ChunkColumn*> activeColumns;
	activeColumns.reserve(columnsMap.size());

	//Dodajemy kolumny do wektora
	for (const auto& [coords, column] : columnsMap) {
		activeColumns.push_back(column.get());
	}

	return activeColumns;
}


//Metoda umożliwiająca dodanie bloku w dane miejsce
void World::setBlock(int x, int y, int z, BlockID blockID)
{
	//Zabezpieczamy przed dodaniem bloku poza mapą
	if (y < 0 || y >= config.worldHeightInChunks * Chunk::CHUNK_SIZE) 
	{
		return;
	}

	int columnX;
	int columnZ;
	int localX;
	int localZ;

	//Przeliczamy koordynaty na koordynaty kolumn i koordynaty lokalne w kolumnie
	getLocalCoords(x, z, columnX, columnZ, localX, localZ);

	//Pobieramy wskaźnik na kolumne 
	if (auto column = getChunkColumn(columnX, columnZ))
	{
		//ustawiamy blok w kolumnie
		column->setBlock(localX, y, localZ, blockID);
	}
}

// METODA ODPOWIADA ZA PONOWNE RENDEROWANIE CHUNKOW KTORE ZOSTALY ZMIENIONE PRZEZ UZYTKOWNIKA -> CZYLI TAKIE NA KTORYM ZNISZCZONO LUB POSTAWIONO BLOK
void World::renderAlteredChunks(std::shared_ptr<ChunkColumn> column)
{

	//Zlecamy zadanie przebudowy siatki dla zaktualizowanego chunka na jednym z wątków
	enqueueTask([this, column]() {
		{
			std::lock_guard<std::mutex> meshLock(column->getMeshMutex());
			column->buildMeshFromPendingData(*this);
		}


		{
			//Po wygenerowaniu danych na CPU dodajemy chunka do kolejki wysyłania do GPU
			std::lock_guard<std::mutex> lock(uploadQueueMutex);
			uploadToGPUQueue.push_back(column);
		}

		});
}


//Metoda odpowiadająca z pobieranie id bloku
BlockID World::getBlock(int x, int y, int z) const
{
	//Zabezpieczamy przed sprawdzeniem bloku poza mapą
	if (y < 0 || y >= config.worldHeightInChunks * Chunk::CHUNK_SIZE) {
		return 0;
	}

	int columnX;
	int columnZ;
	int localX;
	int localZ;

	//Przeliczamy koordynaty na koordynaty kolumn i koordynaty lokalne w kolumnie
	getLocalCoords(x, z, columnX, columnZ, localX, localZ);

	//Pobieramy wskaźnik na kolumne 
	if (auto column = getChunkColumn(columnX, columnZ)) {
		//Zwracamy id otrzymanego bloku
		return column->getBlock(localX, y, localZ);
	}

	return 0;
}


void World::updateWorld()
{
	std::vector<std::shared_ptr<ChunkColumn>> chunksToUploadThisFrame;
	bool isQueueEmpty = true;

	{
		std::lock_guard<std::mutex> uploadQueueLock(uploadQueueMutex);
		int chunksProcessedThisFrame = 0;

		//Ograniczamy lcizbę uploadów siatek na klatkę (MAX_CHUNKS_UPLOADED_PER_FRAME)
		//zapobiega to przycinianiu gdy renderują się duża fragmenty terenu na raz
		while (!uploadToGPUQueue.empty() && chunksProcessedThisFrame < MAX_CHUNKS_UPLOADED_PER_FRAME) {
			std::shared_ptr<ChunkColumn> column = uploadToGPUQueue.front();
			uploadToGPUQueue.pop_front();
			column->isMeshUploadPending = false;

			chunksToUploadThisFrame.push_back(column);
			chunksProcessedThisFrame++;
			uploadedChunksCount++;
		}
	}

	//Przesyłamy obliczoną siatkę do GPU (TRZEBA W GŁÓWNYM WĄTKU przez architektóre OpenGL)
	for (std::shared_ptr<ChunkColumn>& column : chunksToUploadThisFrame) {
		column->uploadMeshToGPU();
	}

	//Zmieniamy stan świata (z LOADING na PLAYING) gdy wszystkie chunki zostały przesłane
	if (currentState == WorldState::LOADING && uploadedChunksCount >= totalChunksToGenerate && isQueueEmpty) {
		currentState = WorldState::PLAYING;
		std::cout << "+[WORLD] Zakonczono generowanie swiata!" << std::endl;
	}


	//Dynamiczna ładowanie i usuwanie chunków w trakcie ruchu!!!                                                --------------------------------------------
	if (currentState == WorldState::PLAYING) {

		if(!targetCamera) { return; }

		//Pobieramy pozycje kamery i przeliczamy na koordynaty kolumn (kolumna w której jest kamera)
		int cameraColumnX = static_cast<int>(std::floor(targetCamera->position.x / Chunk::CHUNK_SIZE));
		int cameraColumnZ = static_cast<int>(std::floor(targetCamera->position.z / Chunk::CHUNK_SIZE));

		int chunksQueuedThisFrame = 0;
		std::vector<ChunkCords> chunksToGenerateThisFrame;

		{
			std::shared_lock<std::shared_mutex> columnsMapLock(columnsMapMutex);
			std::lock_guard<std::mutex> generationSetLock(generationSetMutex);

			//Pętla która PROMIENIŚĆIE od gracza wyszukuje nowe kolumny (odległość - lengthFromCamera)
			for (int lengthFromCamera = 0; lengthFromCamera <= config.renderDistance + 1; lengthFromCamera++) {
				for(int x = -lengthFromCamera; x <= lengthFromCamera; x++) {
					for(int z = -lengthFromCamera; z <= lengthFromCamera; z++) {

						//Sprawdzamy jedynie krawędzie aktualnego pierścienie odległośći od kamer
						if(std::abs(x) != lengthFromCamera && std::abs(z) != lengthFromCamera) {
							continue;
						}

						ChunkCords coords = { cameraColumnX + x, cameraColumnZ + z };

						//Sprawdzamy czy kolumna już istnieje
						if (columnsMap.find(coords) != columnsMap.end()) {
							continue;
						}

						//Sprawdzamy czy inny wątek nie genereuje już tej kolumny
						if (columnsCurrentlyGenerating.find(coords) == columnsCurrentlyGenerating.end()) {
							//Jeśli kolumna nie istnieje i nie jest generowana to dodajemy ją do listy zadań w tej klatce
							chunksToGenerateThisFrame.push_back(coords);
							columnsCurrentlyGenerating.insert(coords); //ozanczamy ją jako obecnie generwoaną
							chunksQueuedThisFrame++;

							//Jeśli osiągneliśmy limit w tej klatce to przerwyamy pętle
							if (chunksQueuedThisFrame >= MAX_CHUNKS_GENERATED_PER_FRAME) {
								break;
							}
						}

					}
					if (chunksQueuedThisFrame >= MAX_CHUNKS_GENERATED_PER_FRAME) { break; } //Jeśli osiągneliśmy limit w tej klatce to przerwyamy pętle
				}
				if (chunksQueuedThisFrame >= MAX_CHUNKS_GENERATED_PER_FRAME) { break; } //Jeśli osiągneliśmy limit w tej klatce to przerwyamy pętle
			}
		}

		//Przekazujemy nowe kolumny do wątków roboczych
		for (const auto& coords : chunksToGenerateThisFrame) {
			std::cout << "+[WORLD] Dodano kolumne do generowania: " << coords.x << ", " << coords.z << std::endl;

			enqueueTask([this, coords]() {
				//Przerywamy jeśli wyłączone dalszą generację
				if(!isGenerating) { return; }
				addChunkColumn(coords.x, coords.z);

				//Po zakończeniu generowania bezpiecznie usuwamy kolumny z obecnie generowanych
				std::lock_guard<std::mutex> generationSetLock(generationSetMutex);
				columnsCurrentlyGenerating.erase(coords);
				});
		}

		//Logika usuwania chunków które znalazły się poza zasięgiem render distance (dodajemy troche dystansu gdyby kamera chodziła przód i w tył)
		int unloadDistance = config.renderDistance + 2;
		std::vector<ChunkCords> chunksToUnload;
		{
			std::shared_lock<std::shared_mutex> columnsMapLock(columnsMapMutex);

			//Przeszukujemy załadowane obecnie chunki w pamięci 
			for(const auto& [coords, column] : columnsMap) {
				//sprawdzamy odległość w lini prostej od kolumny w której jest gracz
				if(std::abs(coords.x - cameraColumnX) > unloadDistance || std::abs(coords.z - cameraColumnZ) > unloadDistance) {

					//jeśli kolumna jest dalej niz bezpieczna odległość dodajemy ją do vektora kolumn do usuniecia z pamięci
					chunksToUnload.push_back(coords);
				}
			}
		}

		if (!chunksToUnload.empty()) {
			std::unique_lock<std::shared_mutex> columnsMapLock(columnsMapMutex);
			std::lock_guard<std::mutex> uploadQueueLock(uploadQueueMutex);

			//Przechodzimy przez wszystkie kolumny które trzeba usnuąć z pamięci 
			for (const ChunkCords& coords : chunksToUnload) {
				std::shared_ptr<ChunkColumn> columnPtr = columnsMap[coords];

				//Oznaczamy chunk do usunięcia (wątki w tle mogą dalej je modyfikaować!!!!!)
				columnPtr->isMarkedForDeletion = true;

				//Zapobiegamy wysłaniu chunka który jest oznaczony do usunęcia na GPU
				auto it = std::find(uploadToGPUQueue.begin(), uploadToGPUQueue.end(), columnPtr);
				if (it != uploadToGPUQueue.end()) {
					uploadToGPUQueue.erase(it);
				}
			}
		}

	}


	{
		std::unique_lock<std::shared_mutex> columnsMapLock(columnsMapMutex);

		//Usuwamy wszystkie oznaczone do usunięcia chunki z pamięci
		for (auto it = columnsMap.begin(); it != columnsMap.end(); ) {
			if (it->second->isMarkedForDeletion) {
				it = columnsMap.erase(it);
			}
			else {
				it++;
			}
		}
	}
}



void World::regenerateWorld()
{
	//Bezpiecznie zatrzymujemy obecną generacje

	//zmieniamy flage generowania na false
	//czekamy aż wątki robocze zakończą działanie i je usuwamy
	isGenerating = false;
	if (generationThread.joinable()) {
		generationThread.join();
	}

	while (pendingTasks > 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(3));  //Możliwe że do zmiany                   <-----------------------------------------
	}


	//Sprawdzamy jak wysoko sięgają warstwy aktualnych algorytmów terenu
	if (terrainGenerator && !terrainGenerator->generationLayers.empty()) {
		int maxAlgorithmHeight = 0;

		for (const TerrainLayer& layer : terrainGenerator->generationLayers) {
			if (layer.endY > maxAlgorithmHeight) {
				maxAlgorithmHeight = layer.endY;
			}

			//Aktualizujemy seed dla wszystkich podpietch warst
			if (layer.algorithm) {
				layer.algorithm->setSeed(config.worldSeed);
			}
		}

		//Obliczamy ilość pionowych chunków dodając 1 dla bezpieczeństwa
		int optimalHeightInChunks = (maxAlgorithmHeight / Chunk::CHUNK_SIZE) + 1;
		if (optimalHeightInChunks < 1) { optimalHeightInChunks = 2; }

		config.worldHeightInChunks = optimalHeightInChunks;
		std::cout << "+[WORLD] Optymalna wysokosc: " << config.worldHeightInChunks << "\n";
	}


	//Dla pewności czyścimy wszystkie zasoby jednocześnie je blokując zeby upewnić się że nic nie będzie ich czytać podczas usuwania
	{
		std::unique_lock<std::shared_mutex> columnsMapLock(columnsMapMutex);
		std::lock_guard<std::mutex> uploadQueueLock(uploadQueueMutex);
		std::lock_guard<std::mutex> generationSetLock(generationSetMutex);
		columnsMap.clear();
		uploadToGPUQueue.clear();
		columnsCurrentlyGenerating.clear();
	}

	//Pobieramy koordynaty gracza aby wygenerować nowy teren wokół niego
	int camColX = 0;
	int camColZ = 0;
	if (targetCamera) {
		camColX = static_cast<int>(std::floor(targetCamera->position.x / Chunk::CHUNK_SIZE));
		camColZ = static_cast<int>(std::floor(targetCamera->position.z / Chunk::CHUNK_SIZE));
	}

	//ustawaimy zasięg genearcji dla siatki oraz danych wieszchołków
	int meshRadius = config.renderDistance;
	int dataRadius = config.renderDistance + 1; //musimy zwiekszyc dystans o 1 ponieważ siatka musi miec informacje o wszystkich sąsiadach (Histereza-poczytać!)

	//Reset liczników dla paska ładowania
	totalChunksToGenerate = ((meshRadius * 2) + 1) * ((meshRadius * 2) + 1);
	generatedChunksCount = 0;
	uploadedChunksCount = 0;

	//zmienia stanuświata oraz flagi na łądowanie
	currentState = WorldState::LOADING;
	isGenerating = true;

	//Uruchomienie głównego wątku generatora
	generationThread = std::thread([this, camColX, camColZ, dataRadius, meshRadius]() {

		std::vector<std::future<void>> localFutures;

		//Ustalamy liczbę wątków (jeśli procesor nie będzie miał 4 to system i tak sobie z tym  poradzi przeplatajac działania na 1 wątku)
		unsigned int availableThreads = std::thread::hardware_concurrency();
		if (availableThreads == 0) {
			availableThreads = 4;
		}
		unsigned int workerThreadsCount = 1;
		if (availableThreads > 2) {
			workerThreadsCount = availableThreads - 2;
		}

		//Generowanie bloków (według id)
		//Rodzielamy prace równom,iernie metodą korku przestępnego miedzy dostępne wątki
		for (unsigned int t = 0; t < workerThreadsCount; t++) {
			enqueueTask([this, t, workerThreadsCount, camColX, camColZ, dataRadius]() {

					for (int x = -dataRadius + t; x <= dataRadius; x += workerThreadsCount) {
						for (int z = -dataRadius; z <= dataRadius; z++) {

							if (!isGenerating) { return; }

							ChunkCords coords = {camColX + x, camColZ + z};
							auto column = std::make_shared<ChunkColumn>(coords.x, coords.z);

							//Wypełnianie kolumny bolkami według algorytmu szumu
							if (terrainGenerator) {
								terrainGenerator->applyToColumn(*column);
							}

							//Przeniesienie kolumny do mapy świaata
							{
								std::unique_lock<std::shared_mutex> columnsMapLock(columnsMapMutex);
								columnsMap[coords] = column;
							}
						}

					}
				});
		}

		//Czekamy aż wszytskie kolumny będa miały wygenerowane bloki (musmy to zrobić bo żeby wybudować mesh trzeba sprawdzić swoich sąsiadów)
		while (pendingTasks > 0) {
			if (!isGenerating) { return; }

			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		//Budujemy mesh z gotowych bloków w pamięci przy pomocy Vertexów (wierzchołki)
		for (unsigned int t = 0; t < workerThreadsCount; t++) {
			enqueueTask([this, t, workerThreadsCount, camColX, camColZ, meshRadius]() {

					//Krok przestepny np
					//0, 2, 4, 6
					//1, 3, 5, 7
					for (int x = -meshRadius + t; x <= meshRadius; x += workerThreadsCount) {
						for (int z = -meshRadius; z <= meshRadius; z++) {

							//Jeśli flaga generacji jest ustawiona na false to zaprzestajemy generować
							if (!isGenerating) { return; }

							ChunkCords coords = { camColX + x, camColZ + z };
							std::shared_ptr<ChunkColumn> colPtr = getChunkColumn(coords.x, coords.z);

							if (colPtr) {

								//Blokujemy konkretną kolumnę żeby podczas budowania siatki nie została zmodyfikowana
								{
									std::lock_guard<std::mutex> meshLock(colPtr->getMeshMutex());
									colPtr->buildMeshFromPendingData(*this);
								}

								//Gwarantujemy że kolumna trafi do kolejki na GPU tylko raz
								bool expected = false;
								if(colPtr->isMeshUploadPending.compare_exchange_strong(expected, true)) {
									std::lock_guard<std::mutex> lock(uploadQueueMutex);
									uploadToGPUQueue.push_back(colPtr);
								}

								//Zwiększamy ilość wygenerowanych chunmków dla paska procentowego na LoadinScene
								generatedChunksCount++;
							}
						}
					}

			});
		}
	});

	std::cout << "+[WORLD] Pomyslnie utworzono wszystkie watki" << std::endl;
}