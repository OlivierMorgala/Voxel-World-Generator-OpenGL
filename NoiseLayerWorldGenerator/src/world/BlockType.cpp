#include "world\BlockType.h"

std::vector<BlockData> BlockDatabase::blocksVector;

void BlockDatabase::init()
{
	blocksVector.reserve(256);

	//Inicjalizacja bazy danych bloków - tutaj rejestrujemy podstawowe typy bloków które będą używane w grze
	blocksVector.push_back({ 0, "Air", false, true, glm::vec3(0.0f) }); //NIE ZMIENIAĆ TYEGO BLOKU! ID 0 MUSI BYĆ ZAREZERWOWANE DLA "Air" (Pusty blok)
																		//ponieważ jest używany m.in w klasie Chunk 
	
	blocksVector.push_back({ 1, "Dirt", true, false, glm::vec3(0.35f, 0.24f, 0.15f) });
	blocksVector.push_back({ 2, "Stone", true, false, glm::vec3(0.5f, 0.5f, 0.5f) });


	//TEST PRZEPEŁNIENIA WEKTORA
	/*for (int i = 3; i < 256; i++) {
		std::string testName = "Testowy Blok " + std::to_string(i);
		glm::vec3 testColor = glm::vec3(static_cast<float>(i) / 255.0f, 0.2f, 0.8f);

		registerBlockData(testName, true, false, testColor);
	}*/
}

BlockID BlockDatabase::registerBlockData(const std::string& name, bool isCollidable, bool isTransparent, glm::vec3 color)
{
	if (blocksVector.size() >= 256) {
		std::cout << "[ERROR::BLOCKTYPE] Cannot register new block type -> maximum number of block types (256) reached." << std::endl;
		return 1;
	}

	BlockID newID = static_cast<BlockID>(blocksVector.size());
	blocksVector.push_back({ newID, name, isCollidable, isTransparent, color });
	return newID;
}

const BlockData& BlockDatabase::getBlockData(BlockID id)
{
	// Sprawdzenie czy ID jest poprawne przed zwróceniem danych bloku. Jeśli ID jest poza zakresem wypisujemy błąd i rzucamy wyjątek
	if (id < blocksVector.size()) {
		return blocksVector[id];
	}
	else {
		std::cout << "[ERROR::BLOCKTYPE] Invalid BlockID: " << id << std::endl;
		throw std::out_of_range("Invalid BlockID: " + std::to_string(id));
	}
}

const std::vector<BlockData>& BlockDatabase::getAllBlocks()
{
	return blocksVector;
}
