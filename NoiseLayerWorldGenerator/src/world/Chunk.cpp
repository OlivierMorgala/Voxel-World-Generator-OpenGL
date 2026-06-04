#include "world\Chunk.h"

const int Chunk::neighborOffsets[6][3] = {
		{  0,  0,  1 }, // Przód 
		{  0,  0, -1 }, // Tył
		{ -1,  0,  0 }, // Lewo
		{  1,  0,  0 }, // Prawo
		{  0,  1,  0 }, // Góra
		{  0, -1,  0 }  // Dół
};

const float Chunk::faceVertices[6][12] = {
		{ 0,0,1, 1,0,1, 1,1,1, 0,1,1 }, // Przód
		{ 1,0,0, 0,0,0, 0,1,0, 1,1,0 }, // Tył
		{ 0,0,0, 0,0,1, 0,1,1, 0,1,0 }, // Lewo
		{ 1,0,1, 1,0,0, 1,1,0, 1,1,1 }, // Prawo
		{ 0,1,1, 1,1,1, 1,1,0, 0,1,0 }, // Góra
		{ 0,0,0, 1,0,0, 1,0,1, 0,0,1 }  // Dół
};

const glm::vec2 Chunk::faceCoords[4] = {
	{0.0f, 0.0f}, //lewy dół
	{1.0f, 0.0f}, //prawy dół
	{1.0f, 1.0f}, //prawy góra
	{0.0f, 1.0f}  //lewy góra
};

// Konstruktor domyślny inicjalizujący wskaźnik na tablicę bloków jako nullptr i ustawiający fillBlockID na 0 (co ma reprezentować PUSTY BLOK !!!)
Chunk::Chunk() : blocksTable(nullptr), fillBlockID(0)
{
}

Chunk::~Chunk()
{
}

void Chunk::collectMeshData(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, uint32_t& indexOffset, int chunkYOffset, Chunk* topNeighbor, Chunk* bottomNeighbor, Chunk* frontNeighbor, Chunk* backNeighbor, Chunk* leftNeighbor, Chunk* rightNeighbor) const
{
	// Iterujemy przez wszystkie bloki w sekcji
	for (int z = 0; z < CHUNK_SIZE; z++) {
		for (int y = 0; y < CHUNK_SIZE; y++) {
			for (int x = 0; x < CHUNK_SIZE; x++) {

				// Pobieramy ID aktualnego bloku. Jeśli jest równy 0 (Pusty blok) to pomijamy go i przechodzimy do następnego bloku
				BlockID currentBlockID = getBlock(x, y, z);
				if (currentBlockID == 0) { continue; }

				//Pobieramy dane aktualnego bloku na podstawie jego ID z bazy danych bloków
				const auto& blockData = BlockDatabase::getBlockData(currentBlockID);

				// Dla każdego bloku sprawdzamy jego 6 sąsiadów (przód, tył, lewo, prawo, góra, dół) aby określić które ścianki są widoczne
				for (int wall = 0; wall < 6; wall++) {
					// Obliczamy pozycję sąsiada na podstawie aktualnej pozycji bloku i kierunku sąsiada
					int neighborX = x + neighborOffsets[wall][0];
					int neighborY = y + neighborOffsets[wall][1];
					int neighborZ = z + neighborOffsets[wall][2];

					// Zakładamy że ścianka nie jest widoczna dopóki nie sprawdzimy sąsiada
					bool isFaceVisible = false;

					//Zmienna przechowuje ID bloku sąsiadującego z aktualną ścianą
					BlockID neighborID = 0;

					//Logika sprawdzania widoczności ścianki wzgledem sąsiada
					if (neighborY >= CHUNK_SIZE) {
						//Sąsiad powyżej aktualnego chunka - bierzemy blok z samego dołu (Y=0) górnego sąsiada
						neighborID = getBlockFromNeighbor(topNeighbor, neighborX, 0, neighborZ);
					}
					else if (neighborY < 0) {
						//Sąsiad poniżej aktualnego chunka - bierzemy blok z samej góry (Y=MAX) dolnego sąsiada
						neighborID = getBlockFromNeighbor(bottomNeighbor, neighborX, CHUNK_SIZE - 1, neighborZ);
					}
					else if (neighborX >= CHUNK_SIZE) {
						//Sąsiad po prawej stronie - bierzemy blok z lewej krawędzi (X=0) prawego sąsiada
						neighborID = getBlockFromNeighbor(rightNeighbor, 0, neighborY, neighborZ);
					}
					else if (neighborX < 0) {
						//Sąsiad po lewej stronie - bierzemy blok z prawej krawędzi (X=MAX) lewego sąsiada
						neighborID = getBlockFromNeighbor(leftNeighbor, CHUNK_SIZE - 1, neighborY, neighborZ);
					}
					else if (neighborZ >= CHUNK_SIZE) {
						//Sąsiad z przodu - bierzemy blok z tyłu (Z=0) przedniego sąsiada
						neighborID = getBlockFromNeighbor(frontNeighbor, neighborX, neighborY, 0);
					}
					else if (neighborZ < 0) {
						//Sąsiad z tyłu - bierzemy blok z przodu (Z=MAX) tylnego sąsiada
						neighborID = getBlockFromNeighbor(backNeighbor, neighborX, neighborY, CHUNK_SIZE - 1);
					}
					else {
						//Sąsiad znajduje się wewnątrz tego samego chunka - pobieramy go bezpośrednio
						neighborID = getBlock(neighborX, neighborY, neighborZ);
					}


					//Decyzja o rysowaniu ścianki na podstawie otrzymanego bloku sąsiada
					if (neighborID == 0 || BlockDatabase::getBlockData(neighborID).isTransparent) {
						//Jeśli sąsiad jest pustym blokiem (ID=0) lub jest blokiem przezroczystym
						isFaceVisible = true;
					}


					// Jeśli ścianka jest widoczna to dodajemy jej wierzchołki do wektora wierzchołków
					if (isFaceVisible) {

						// Dodajemy wierzchołki dla tej ścianki. Współrzędne wierzchołków są obliczane na podstawie lokalnych współrzędnych ścianki (faceVertices) i globalnej pozycji bloku (x, y, z)
						for (int v = 0; v < 4; v++) {
							//Tworzymy strukturę wierzchołka i wypełniamy ją danymi
							Vertex vertex;

							// Obliczamy globalną pozycję wierzchołka dodając lokalne współrzędne ścianki do pozycji bloku. Dodajemy również chunkYOffset do współrzędnej Y aby uwzględnić wysokość chunka w świecie
							vertex.position = glm::vec3(
								x + faceVertices[wall][v * 3 + 0],
								y + chunkYOffset + faceVertices[wall][v * 3 + 1],
								z + faceVertices[wall][v * 3 + 2]
							);
							// Ustawiamy kolor wierzchołka na podstawie koloru bloku z bazy danych bloków
							vertex.color = blockData.color;
							// Ustawiamy lokalną pozycję by shader mógł poprawnie nałożyć teksturę ramki na ściankę
							vertex.localPosition = faceCoords[v];

							vertices.push_back(vertex);
						}

						// Dodajemy indeksy dla tej ścianki (2 trójkąty tworzące kwadrat)
						indices.push_back(indexOffset + 0);
						indices.push_back(indexOffset + 1);
						indices.push_back(indexOffset + 2);

						indices.push_back(indexOffset + 2);
						indices.push_back(indexOffset + 3);
						indices.push_back(indexOffset + 0);

						// Aktualizujemy offset indeksów o 4 ponieważ każda ścianka dodaje 4 wierzchołki
						indexOffset += 4;
					}

				}
			}
		}
	}
}

void Chunk::setBlock(int x, int y, int z, BlockID blockID)
{
	if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) { return; }

	//Sprawdzamy czy tablica bloków jest już zainicjalizowana
	if (blocksTable == nullptr) {
		//Jeśli nie jest a ID bloku który chcemy ustawić jest taki sam jak fillBlockID to
		// znaczy że nie musimy tworzyć tablicy bo cały chunk jest już wypełniony tym blokiem
		if (blockID == fillBlockID) { return; }

		//Jeśli ID bloku jest różne od fillBlockID to musimy stworzyć tablicę i wypełnić ją wartością fillBlockID
		blocksTable = std::make_unique<BlockID[]>(CHUNK_VOLUME);
		for(int i = 0; i < CHUNK_VOLUME; i++) {
			blocksTable[i] = fillBlockID;
		}
	}

	//Ustawiamy ID bloku w tablicy na podstawie jego pozycji w sekcji. Indeks obliczamy jako (x + y*16 + z*256)
	blocksTable[x + (y * CHUNK_SIZE) + (z * CHUNK_SIZE * CHUNK_SIZE)] = blockID;
}

BlockID Chunk::getBlock(int x, int y, int z) const
{
	//Sprawdzamy czy podane współrzędne są poza granicami sekcji. Jeśli tak to zwracamy 0 (PUSTY BLOK). Musimy to zrobić ponieważ metoda ta jest używana przez metodę generateMesh
	if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) { return 0; }

	//Sprawdzamy czy tablica bloków jest zainicjalizowana. Jeśli nie jest to znaczy że
	// cały chunk jest wypełniony blokiem o ID fillBlockID więc zwracamy tę wartość
	if (blocksTable == nullptr) { return fillBlockID; }

	//Zwracamy ID bloku z tablicy na podstawie jego pozycji w sekcji. Indeks obliczamy jako (x + y*16 + z*256)
	return blocksTable[x + (y * CHUNK_SIZE) + (z * CHUNK_SIZE * CHUNK_SIZE)];
}

BlockID Chunk::getBlockFromNeighbor(Chunk* neighbor, int x, int y, int z) const
{
	// Jeśli sąsiad istnieje to pobieramy ID bloku z tego sąsiada na podstawie podanych współrzędnych. Jeśli sąsiad nie istnieje to zwracamy 0 (PUSTY BLOK)
	if (neighbor) {
		return neighbor->getBlock(x, y, z);
	}
	return 0;
}

