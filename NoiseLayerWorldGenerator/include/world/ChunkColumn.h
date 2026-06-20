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
	//Koordynaty kolumny w świecie
	int columnX;
	int columnZ;

	//Dwa oddzielne obiekty Mesh: jeden dla bloków nieprzeźroczystych drugi dla przeźroczystych
	std::unique_ptr<Mesh> opaqueColumnMesh;
	std::unique_ptr<Mesh> transparentColumnMesh;

	//Flagi określające stan siatki(mesh)
	bool isMeshGenerated = false;
	bool isRerenderNeeded = false;
	bool hasPendingMeshData = false;

	//Tymczasowe bufory przechowujące dane wierzchołków obliczone na procesorze, czekającae na upload do VBO
	std::vector<Vertex> pendingOpaqueVertices;	//wierzchołki 
	std::vector<uint32_t> pendingOpaqueIndices;	//indeksy (żeby nie powtarzać wierzchołków patrz mesh.cpp)
	std::vector<Vertex> pendingTransparentVertices;
	std::vector<uint32_t> pendingTransparentIndices;

	//Mutex zabezpieczający dane siatki przed wyścigiem miedzy głównymi wątkami (renderowanie) a roboczymi (budowanie)
	std::mutex meshMutex;

	//Wektor przechowujący pojedyńcze chunki (16x16x16) dla danej kolumny
	std::vector<std::unique_ptr<Chunk>> chunks;

public:
	ChunkColumn(int x, int z);
	~ChunkColumn() = default;

	//Flagi atomowe wykorzystywane do bezpiecznego wielowątkowego zarządzania cyklem życia kolumny
	std::atomic<bool> isMarkedForDeletion = false;
	std::atomic<bool> isMeshUploadPending = false;

	//Mtody do ustawiania oraz pobierania danych bloku
	void setBlock(int x, int y, int z, BlockID blockID);
	BlockID getBlock(int x, int y, int z) const;

	//Zwraca wskaźnik konkretnego chunka w kolumnie z jego pozycji w pionie
	Chunk* getChunk(int yIndex) const;

	//Obliczenie danych wierzchołków dla całej kolumny
	void buildMeshFromPendingData(const World& world);
	//Przypisanie przygotowanych wyżej wektorów do obiektów VBO.VAO/EBO karty graficznej
	void uploadMeshToGPU();

	//Renderowanie chunków(najpierw przeźroczyste potem nieprzeźroczyste)
	void renderOpaque(Shader* shader) const;
	void renderTransparent(Shader* shader) const;

	//Metody zwracające koordynaty kolumny w mapie kolumn
	int getX() const;
	int getZ() const;

	//Zwraca referencej do mutexa siatki
	//Pozwala to klasie WORLD "zablokować" tylko tę konkretną kolumnę dla innych wątków na czas
	//gdy jeden z nich przebudowywuje jej wektory wierzchołków
	std::mutex& getMeshMutex();

	//Sprawdza czy kolumna ma już wygenerowaną i przesłaną na GPU siatkę
	bool hasMesh() const;

	//Informuje czy siatka kolumny wymaga przerobienia
	bool needsRerender() const;
};

