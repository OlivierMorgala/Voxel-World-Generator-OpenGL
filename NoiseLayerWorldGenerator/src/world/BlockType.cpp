#include "world\BlockType.h"

std::vector<BlockData> BlockDatabase::blocksVector;

void BlockDatabase::init()
{
	blocksVector.reserve(256);

	//Inicjalizacja bazy danych bloków - tutaj rejestrujemy podstawowe typy bloków które będą używane w grze
	blocksVector.push_back({ 0, "Air", false, true, glm::vec3(0.0f) }); //NIE ZMIENIAĆ TYEGO BLOKU! ID 0 MUSI BYĆ ZAREZERWOWANE DLA "Air" (Pusty blok)
																		//ponieważ jest używany m.in w klasie Chunk 
	
	blocksVector.push_back({ 1, "Dirt", true, false, glm::vec3(0.35f, 0.24f, 0.15f) });
	blocksVector.push_back({ 2, "Stone", true, false, glm::vec3(0.28f, 0.28f, 0.28f) });
	blocksVector.push_back({ 3, "Grass", true, false, glm::vec3(0.18f, 0.55f, 0.13f) }); 
	blocksVector.push_back({ 4, "Sand", true, false, glm::vec3(0.92f, 0.86f, 0.65f) });
	blocksVector.push_back({ 5, "Snow", true, false, glm::vec3(0.95f, 0.98f, 1.0f) }); 
	blocksVector.push_back({ 6, "DarkRock", true, false, glm::vec3(0.12f, 0.12f, 0.14f) });
	blocksVector.push_back({ 7, "Marble", true, false, glm::vec3(0.85f, 0.85f, 0.82f) });
	blocksVector.push_back({ 8, "DarkDirt", true, false, glm::vec3(0.20f, 0.12f, 0.05f) });
	blocksVector.push_back({ 9, "Clay", true, false, glm::vec3(0.55f, 0.58f, 0.65f) });
	blocksVector.push_back({ 10, "GoldOre", true, false, glm::vec3(0.85f, 0.70f, 0.20f) });
	blocksVector.push_back({ 11, "DiamondOre", true, false, glm::vec3(0.25f, 0.88f, 0.82f) });
	blocksVector.push_back({ 12, "Lapis", true, false, glm::vec3(0.10f, 0.35f, 0.75f) });
	blocksVector.push_back({ 13, "EmeraldOre", true, false, glm::vec3(0.10f, 0.85f, 0.35f) }); 
	blocksVector.push_back({ 14, "Wood", true, false, glm::vec3(0.40f, 0.28f, 0.15f) });
	blocksVector.push_back({ 15, "Water", false, true, glm::vec3(0.10f, 0.30f, 0.95f) });
	blocksVector.push_back({ 16, "Lava", true, true, glm::vec3(0.95f, 0.35f, 0.0f) });


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
