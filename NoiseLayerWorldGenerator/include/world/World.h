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
#include <unordered_map>
#include <algorithm>

#include "WorldConfig.h"

//Limity operacji na jedną klatkę (dla zpobiegania gwałtownym spakom FPS przy poruszaniu)
constexpr int MAX_CHUNKS_GENERATED_PER_FRAME = 2;
constexpr int MAX_CHUNKS_UPLOADED_PER_FRAME = 8;

//Enum stanów świata określa czy świat się ładuje podczas ekranu łądowania czy normalnie podczas sceny 
enum class WorldState {
	PLAYING,
	LOADING
};

//Struct odpowiadający za współrzędne 2D kolumn w świecie
struct ChunkCords {
    int x, z;

    // Musimy przeciążyć operator <, aby std::map wiedziała jak sortować klucze (wartości po których szuka)
    bool operator<(const ChunkCords& other) const {
        if (x != other.x) { return x < other.x; }
        return z < other.z;
    }

    // Operator porównania potrzebny do weryfikacji równości współrzędnych (wymagany przy unordered_set i unordered_map)
    bool operator==(const ChunkCords& other) const {
        return x == other.x && z == other.z;
    }
};

//Struktura hashująca dla !ChunkCords! niezbędna do używania jako klucza w wszytkich klasach typu unordered
struct ChunkCordsHash {
    std::size_t operator()(const ChunkCords& cords) const {
        return std::hash<int>()(cords.x) ^ (std::hash<int>()(cords.z) << 1);
    }
};

class World
{
private:
    //Aktualny stan świata
	WorldState currentState = WorldState::PLAYING;

    //Struktury do zarządzania wątkami
	std::vector<std::thread> workerThreads;  //Wektor wątków
	std::queue<std::function<void()>> tasksQueue; //Kolejka zadań do wykonania
	std::mutex tasksQueueMutex;  //Mutex zabezpieczający dostęp do kolejki zadań
    std::condition_variable condition;  //Zmienna warunku która służy do zarządzania stanem wątku (aktywacaja/deaktywacaja)
    std::atomic<bool> stopPool;  //Flaga która informuje wątek o konieczności zakończenia pracy

    std::thread generationThread;  //Specjlany wątek dedykowany wyłącznie do zarządzania początkowym generowaniem świata
    std::atomic<int> pendingTasks = 0;  //Licznik zliczający aktualnie oczekujące zadania

	std::atomic<bool> isGenerating = false;  //Flaga określająca czy świat się obecnie generuje

    //Zmienne do obliczania paska postępu generacji
	std::atomic<int> generatedChunksCount = 0; //Zmienna licząca ile siatek chunków zostało obliczonych 
	std::atomic<int> uploadedChunksCount = 0; //Zmienna licząca ile siatek chunków zostało przesłanych do GPU

    //Całkowita liczba chunków do wygenerowania
	int totalChunksToGenerate = (2 * config.renderDistance + 1) * (2 * config.renderDistance + 1);
    
    //Kolejka wskaźników na kolumny które mają gotowe siatki i czekają na wysłąnie do GPU
	std::deque<std::shared_ptr<ChunkColumn>> uploadToGPUQueue;
	mutable std::mutex uploadQueueMutex;

    //Zbiór przechowujący współrzędne kolumn któree są aktualnie w trakcie generacji (głównie po to żeby nie liczyć kolumn więcej razy)
    std::unordered_set<ChunkCords, ChunkCordsHash> columnsCurrentlyGenerating;
	std::mutex generationSetMutex;

    //Wskaźnik na kamere gracza
	const Camera* targetCamera = nullptr;
    //Wskaźnik na obiekt odpowiedzialny za algorytmy generujące teren
    WorldTerrainGenerator* terrainGenerator = nullptr;

    //Główna mapa przechowująca wszystkie załadowane kolumny chunków
    //Używamy shered_mutex aby umożliwić jednoczesny odczyt przez wiele wątków (zapis jak coś jest dalej jedynie dla jednego wątku!!!!)
    std::unordered_map<ChunkCords, std::shared_ptr<ChunkColumn>, ChunkCordsHash> columnsMap;
    mutable std::shared_mutex columnsMapMutex;

    //Metoda która dodaje nowe zadania do kolejki dla wątków
    void enqueueTask(std::function<void()> task);

    //Metoda pomocnicza przeliczająca globalne wspólrzędne bloku na współrzędne kolumny i lokalne współrzędne bloku wewnątrz chunka
	void getLocalCoords(int globalX, int globalZ, int& columnX, int& columnZ, int& localX, int& localZ) const;

public:
    World();
    ~World();

    //Metoda zwracający obecny stan świata generuje/aktywny
    WorldState getCurrentState() const;
    //Metoda zwracające postęp generacji świata (dla generowania początkowego)
    float getGenerationProgress() const;

    //Metody ustawiające obecną kamerę oraz obecny generator świata z których korzysta obiekt
	void setCamera(const Camera* camera);
    void setTerrainGenerator(WorldTerrainGenerator* generator);

    //Modyfikacja i pobieranie danych konkretenego bloku w swiecie
    void setBlock(int x, int y, int z, BlockID blockID);
    BlockID getBlock(int x, int y, int z) const;

    //Metdoy do zarządzania kolumnami chunków
    void addChunkColumn(int x, int z);
    std::shared_ptr<ChunkColumn> getChunkColumn(int x, int z) const;
    std::vector<ChunkColumn*> getLoadedColumns() const;


    void updateWorld(); //Metoda aktualizująca obecny stań świata
    void regenerateWorld(); //Metoda odpowiadająca ze regeneracje świata od podstaw

    //Metoda zwracająca ilosć obecnie załadowanych kolumn
    int getLoadedChunkColumnsCount();

    //Metoda wymuszająca odświerzenie siatki po modyfikacji PRZEZ GRACZA
    void renderAlteredChunks(std::shared_ptr<ChunkColumn> column);
};

