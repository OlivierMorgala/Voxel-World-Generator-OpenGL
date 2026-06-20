#pragma once
#include <string>
#include <vector>
#include <imgui.h>
#include "world/BlockType.h"


// enum BlendMode TRYB Mieszania Warstw: opcje te okreslaja jak nowa warstwa ma wplynac na warstwe ktora juz tam byla
enum class BlendMode {
	NORMAL,
	ADD,
	SUBTRACT,
	MULTIPLY,
	MAX,
	MIN,
	SMOOTH,
	ABSOLUTE,
	CARVE,
	CARVEIN
};


// Klasa TerrainModifier: Klasa ktora obsluguje filtry ktore mozna nakladac na algorytmy
class TerrainModifier
{
public:
	virtual ~TerrainModifier() = default;

	virtual float modify(float value) = 0;
	virtual void renderImGui() = 0;
};

// Klasa TerrainAlgorithm: Klasa bazowa dla wszystkich algorytmow, kazdy algorytm generowania terenu po niej dziedziczy
class TerrainAlgorithm {
public:
	virtual ~TerrainAlgorithm() = default;

	virtual float evaluate(float x, float z) = 0; // Metoda zwraca wartosc, ktora potem wplywa na wysokosc terenu podajac koordynanty x i z
	virtual float evaluate3D(float x, float y, float z) { return 0.0f; }; // Metoda zwraca wartosc w punkcie przestrzeni (x,y,z)
    
	virtual void renderImGui() = 0; // Rysowanie suwakow w ImGui dla ustawien algorytmu
	virtual void setSeed(int newSeed) = 0; // Metoda ustawia seed swiata zeby algorytm zawsze generowal swiat w taki sam sposob dla danego seedu 
};


// Klasa TerrainLayer: Finalna klasa ktora spina wszystko w calosc.
// Zawiera algorytm, filtry, blend mode, ID bloku z ktorego ma powstac warstwa
class TerrainLayer {
public:
	std::string name;
	bool isEnabled = true;

	BlendMode blendMode = BlendMode::ABSOLUTE;

	BlockID targetBlockID = 1; //Tylko dla CARVEIN
	float blendWeight = 0.5f; //Tylko dla SMOOTH i CARVE

	int startY;
	int endY;
	BlockID blockID;

	//Wskaznik na algorytm, ktory generuje ta warstwe
	std::unique_ptr<TerrainAlgorithm> algorithm;

	//vector przechowujacy aktualnie aktywne modyfikatory/filtry nalozone na te warstwe
	std::vector<std::unique_ptr<TerrainModifier>> activeModifiers;

	//Konstruktor TerrainLayer 
 	TerrainLayer(std::string name, int startY, int endY, BlockID blockID, std::unique_ptr<TerrainAlgorithm> algorithm);

};

