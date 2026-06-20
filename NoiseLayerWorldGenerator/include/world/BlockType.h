#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>

// WAŻNE!!!  ---   Definicja typu ID dla bloków używamy uint16_t co pozwala na maksymalnie 65536 różnych typów bloków
using BlockID = uint8_t;

//Struktura przechowująca dane o bloku
struct BlockData
{
	BlockID id;
	std::string name;
	bool isCollidable;
	bool isTransparent;
	glm::vec3 color;

	//Można dodać więcej właściwości bloku w przyszłości, takich jak tekstura itp.
};

class BlockDatabase
{
private:
	// Wektor przechowujący dane wszystkich zarejestrowanych bloków indeksowany przez ID bloku
	static std::vector<BlockData> blocksVector;

public:
	// Inicjalizacja bazy danych bloków ta metoda powinna być wywołana przy generowaniu świata przed jakimkolwiek użyciem danych bloków aby zapewnić że baza jest poprawnie wypełniona
	static void init();

	// Metoda rejestrująca nowy typ bloku i zwracająca jego ID.
	static BlockID registerBlockData(const std::string& name, bool isCollidable, bool isTransparent, glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f));

	//Metoda zwracająca dane bloku na podstawie jego ID
	static const BlockData& getBlockData(BlockID id);

	static const std::vector<BlockData>& getAllBlocks();
};