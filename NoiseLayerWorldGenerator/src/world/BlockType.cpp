#include "world\BlockType.h"

std::vector<BlockData> BlockDatabase::blocksVector;

void BlockDatabase::init()
{
	//Inicjalizacja bazy danych bloków - tutaj rejestrujemy podstawowe typy bloków które będą używane w grze
	blocksVector.push_back({ 0, "Air", false, true, glm::vec3(0.0f) }); //NIE ZMIENIAĆ TYEGO BLOKU! ID 0 MUSI BYĆ ZAREZERWOWANE DLA "Air" (Pusty blok)
													   //ponieważ jest używany m.in w klasie Chunk 
	
	blocksVector.push_back({ 1, "Dirt", true, false, glm::vec3(0.35f, 0.24f, 0.15f) });
	blocksVector.push_back({ 2, "Stone", true, false, glm::vec3(0.5f, 0.5f, 0.5f) });
}

BlockID BlockDatabase::registerBlockData(const std::string& name, bool isCollidable, bool isTransparent)
{
	BlockID newID = static_cast<BlockID>(blocksVector.size());
	blocksVector.push_back({ newID, name, isCollidable, isTransparent });
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
