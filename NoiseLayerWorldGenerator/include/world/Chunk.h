#pragma once
#include "BlockType.h"
#include "Mesh.h"
#include <memory>
#include <vector>
#include <glm/glm.hpp>

class Chunk
{
private:
	// Kierunki sąsiadów (X, Y, Z)
	static const int neighborOffsets[6][3];

	// Wierzchołki dla każdej z 6 ścianek - (WSPÓŁRZĘDNE LOKALNE!)
	static const float faceVertices[6][12];

	// Mapowanie rogów ramki
	static const glm::vec2 faceCoords[4];

	// Tablica przechowująca ID bloków w sekcj
	std::unique_ptr<BlockID[]> blocksTable;
	// ID bloku który będzie używany do wypełniania jeśli sekcja jest jednolita
	BlockID fillBlockID;

	// Funkcja pomocnicza do pobierania ID bloku z sąsiedniej sekcji.
	// Ta funkcja jest używana podczas generowania siatki aby sprawdzić widoczność ścianek na granicy sekcji
	BlockID getBlockFromNeighbor(Chunk* neighbor, int x, int y, int z, BlockID fallbackBlockID) const;

public:
	Chunk();
	~Chunk();

	//Rozmiar sekcji zawsze będzie 16x16x16, co daje 4096 bloków
	const static int CHUNK_SIZE = 16;
	const static int CHUNK_VOLUME = 4096;

	void collectMeshData(std::vector<Vertex>& opaqueVertices, std::vector<uint32_t>& opaqueIndices, uint32_t& opaqueIndexOffset,
		std::vector<Vertex>& transparentVertices, std::vector<uint32_t>& transparentIndices, uint32_t& transparentIndexOffset,
		int chunkYOffset, Chunk* topNeighbor, Chunk* bottomNeighbor, Chunk* frontNeighbor, Chunk* backNeighbor, Chunk* leftNeighbor, Chunk* rightNeighbor) const;

	// Funkcja ustawiająca ID bloku na podstawie jego pozycji w sekcji
	void setBlock(int x, int y, int z, BlockID blockID);
	// Funkcja zwracająca ID bloku na podstawie jego pozycji w sekcji
	BlockID getBlock(int x, int y, int z) const;

};
