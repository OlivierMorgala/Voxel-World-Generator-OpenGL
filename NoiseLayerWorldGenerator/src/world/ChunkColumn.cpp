#include "world/ChunkColumn.h"
#include "world/World.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "world/WorldConfig.h"


ChunkColumn::ChunkColumn(int x, int z) : columnX(x), columnZ(z)
{
	//Alokujemy pamieć dla wektora ponieważ można ją łatwo obliczyć znając wysokośc świata
	chunks.reserve(config.worldHeightInChunks);

	//Wypełnienie kolumny wskaźnikami na obiekty Chunk
	for (int i = 0; i < config.worldHeightInChunks; i++) {
		chunks.push_back(std::make_unique<Chunk>());
	}
}



void ChunkColumn::setBlock(int x, int y, int z, BlockID blockID)
{
	// Zabezpieczenie przed próbą postawienia bloku poza mapą
	if (y < 0 || y >= chunks.size() * Chunk::CHUNK_SIZE) { return; }
	if (x < 0 || x >= Chunk::CHUNK_SIZE || z < 0 || z >= Chunk::CHUNK_SIZE) { return; }

	//Określenie do którego dokładnie piętra (pozycja y w kolumnie) i pozycja y w lokalnych współrzędnych trafia operacja
	int chunkIndex = y / Chunk::CHUNK_SIZE;
	int localY = y % Chunk::CHUNK_SIZE;

	//Zmieniamy blok na podanej pozycji
	chunks[chunkIndex]->setBlock(x, localY, z, blockID);

	//Ustawaimy flage że kolumna musi nie wyrenderować od nowa
	isRerenderNeeded = true;
}



BlockID ChunkColumn::getBlock(int x, int y, int z) const
{
	// Zabezpieczenie przed próbą odczytania bloku poza mapą
	if (y < 0 || y >= chunks.size() * Chunk::CHUNK_SIZE) { return 0; }
	if (x < 0 || x >= Chunk::CHUNK_SIZE || z < 0 || z >= Chunk::CHUNK_SIZE) { return 0; }

	//Określenie do którego dokładnie piętra (pozycja y w kolumnie) i pozycja y w lokalnych współrzędnych trafia operacja
	int chunkIndex = y / Chunk::CHUNK_SIZE;
	int localY = y % Chunk::CHUNK_SIZE;

	//Zwracamy id znalezionego bloku
	return chunks[chunkIndex]->getBlock(x, localY, z);
}


//Metoda zwracająca wskaźnik na chunk na danym piętrze kolumny
Chunk* ChunkColumn::getChunk(int yIndex) const 
{
	if (yIndex >= 0 && yIndex < chunks.size()) {
		return chunks[yIndex].get();
	}
	return nullptr;
}



void ChunkColumn::buildMeshFromPendingData(const World& world)
{
	// Czyszczenie tymczasowych wektorów przygotowując miejsce na nową geometrię
	pendingOpaqueVertices.clear();
	pendingTransparentVertices.clear();
	pendingOpaqueIndices.clear();
	pendingTransparentIndices.clear();
	uint32_t opaqueIndexOffset = 0;
	uint32_t transparentIndexOffset	 = 0;

	//Rezerwujemy ustaloną ilośc pamięci na wierzchołki 
	pendingOpaqueVertices.reserve((Chunk::CHUNK_VOLUME * chunks.size()) / 2);
	pendingOpaqueIndices.reserve(Chunk::CHUNK_VOLUME * chunks.size());

	pendingTransparentVertices.reserve((Chunk::CHUNK_VOLUME * chunks.size()) / 4);
	pendingTransparentIndices.reserve((Chunk::CHUNK_VOLUME * chunks.size()) / 2);


	//Pobieramy sąsiadujące kolumny w celu obsłużenia cullingu na granicach kolumn. Unikamy renderowania ścian, 
	ChunkColumn* frontColumn = world.getChunkColumn(columnX, columnZ + 1);
	ChunkColumn* backColumn = world.getChunkColumn(columnX, columnZ - 1);
	ChunkColumn* leftColumn = world.getChunkColumn(columnX - 1, columnZ);
	ChunkColumn* rightColumn = world.getChunkColumn(columnX + 1, columnZ);

	for (int i = 0; i < chunks.size(); i++) {

		//Pobieram  wskaźniki na sąsiadów konkretnego !CHUNKA! z każdej storny (6)
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

		//Przesunięcie wysokości całego CHUNKA
		int chunkYOffset = i * Chunk::CHUNK_SIZE;

		//Zbiermy wierzchołki z sąsiadujących obiektów Chunk
		chunks[i]->collectMeshData(pendingOpaqueVertices, pendingOpaqueIndices, opaqueIndexOffset, pendingTransparentVertices, pendingTransparentIndices, transparentIndexOffset, chunkYOffset, topNeighbor, bottomNeighbor, frontNeighbor, backNeighbor, leftNeighbor, rightNeighbor);
	}

	//Ozaczamy ze CPU skończyło prace i dane można wysłąć do GPU
	hasPendingMeshData = true;
}



void ChunkColumn::uploadMeshToGPU()
{
	std::lock_guard<std::mutex> meshLock(meshMutex);

	if (!hasPendingMeshData) { return; }

	//Wgrywanie geometri nieprzeżroczystej
	if (!pendingOpaqueVertices.empty()) {

		if (!opaqueColumnMesh) {
			//Tworzymy nową siatke (mesh) jeśli to pierwsza genearcaja
			opaqueColumnMesh = std::make_unique<Mesh>(pendingOpaqueVertices, pendingOpaqueIndices);
		}
		else 
		{
			//Jeśli siatka istnieje podmieniamy dane (glBufferData)
			opaqueColumnMesh->updateData(pendingOpaqueVertices, pendingOpaqueIndices);
		}

	}
	else if (opaqueColumnMesh) {
		//Jeśli nowy chunk jest całkowicie pusty to całkowicie czyścimy buffory
		opaqueColumnMesh->updateData(pendingOpaqueVertices, pendingOpaqueIndices);
	}

	//Wgrywanie geometri przeżroczystej
	if (!pendingTransparentVertices.empty()) {

		if (!transparentColumnMesh) {
			//Tworzymy nową siatke (mesh) jeśli to pierwsza genearcaja
			transparentColumnMesh = std::make_unique<Mesh>(pendingTransparentVertices, pendingTransparentIndices);
		}
		else
		{
			//Jeśli siatka istnieje podmieniamy dane (glBufferData)
			transparentColumnMesh->updateData(pendingTransparentVertices, pendingTransparentIndices);
		}

	}
	else if (transparentColumnMesh) {
		//Jeśli nowy chunk jest całkowicie pusty to całkowicie czyścimy buffory
		transparentColumnMesh->updateData(pendingTransparentVertices, pendingTransparentIndices);
	}


	//Zwalniamy pamięć wektorów tymczasowych po stronie CPU (wszystkie dane są już na GPU)
	pendingOpaqueVertices.clear();
	pendingOpaqueIndices.clear();

	pendingTransparentVertices.clear();
	pendingTransparentIndices.clear();

	//Ustawaimy odpowiednie flagi
	hasPendingMeshData = false;
	isMeshGenerated = true;
	isRerenderNeeded = false;
}



void ChunkColumn::renderOpaque(Shader* shader) const
{
	if (!opaqueColumnMesh) { return; }

	//Przesuwamy całą kolumnę na jej właściwe miejsce w świecie po stronie GPU
	//Bloki mają jedynie współrzędne lokalne dlatego zamiast przeliczać współrzędne dla każdego bloku
	//tworzymy macierz jednostkową i instrukcją translacji obliczamy przesunięcie o koordynaty chunka razy jego rozmiar
	//co daje ralne koordynaty kolumny
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(columnX * Chunk::CHUNK_SIZE, 0, columnZ * Chunk::CHUNK_SIZE));
	//Przekazujemy dane do shadera jako zmienną uniform a shader dzieki macierzy model przesunie każdy wierzchołek na ekranie
	shader->setMatrix4("model", model);

	//Wydanie polecenia aby karta narysowała wierzchołki w danych koordynatach
	opaqueColumnMesh->draw();
}



void ChunkColumn::renderTransparent(Shader* shader) const
{
	if (!transparentColumnMesh) { return; }

	//Obliczamy macierz przecunięcia całej kolumny w świecie
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(columnX * Chunk::CHUNK_SIZE, 0, columnZ * Chunk::CHUNK_SIZE));
	//Przekazujemy dane do shadera jako zmienną uniform a shader dzieki macierzy model przesunie każdy wierzchołek na ekranie
	shader->setMatrix4("model", model);

	//Wydanie polecenia aby karta narysowała wierzchołki w danych koordynatach
	transparentColumnMesh->draw();
}


//Metoda zwarcająca pozycje X chunka w siatce kolumn
int ChunkColumn::getX() const 
{ 
	return columnX; 
}


//Metoda zwarcająca pozycje Z chunka w siatce kolumn
int ChunkColumn::getZ() const 
{ 
	return columnZ; 
}


//Zwraca referencej do mutexa siatki
std::mutex& ChunkColumn::getMeshMutex() 
{
	return meshMutex;
}


//Sprawdza czy kolumna ma już wygenerowaną i przesłaną na GPU siatkę
bool ChunkColumn::hasMesh() const 
{
	return isMeshGenerated;
}


//Informuje czy siatka kolumny wymaga przerobienia
bool ChunkColumn::needsRerender() const 
{
	return isRerenderNeeded;
}