
#include "world/generationAlgorithms/GenerationAlgorithm.h"
#include<cmath>
#include<algorithm>
#include<vector>
#include<random>
#include<numeric>
#include<iostream>
#include<iomanip>



struct vector2
{
	float x, y;
};




class SimplexNoise : public GenerationAlgorithm
{
private:
	std::vector<int> PermutationSimplex;
public:
	float frequencySimplexNoise;
	float amplitdueSimplexNoise;
	int octavesSimplexNoise;
	float frequencyChangeSimplex;
	float amplitudeChangeSimlpex;
	int seedSimplexNoise;

	// Implementacja metod wirtualnych
	virtual void applyToColumn(ChunkColumn& column) override;
	virtual void renderImGuiSettings() override;

	// Funkcje Noise
	float DotProduct(float gx, float gy, float x, float y);
	SimplexNoise(std::string Name, int startY, int endY, int seedSimpexNoise, float frequency, float amplitude, int octaves, float freqchange, float ampchange);
	float simplexNoise(float x, float y);
	int Hash(int x, int y);
	float OctavesSimplexFunction(float x, float y, float frequency, float amplitude, int octaves, float frequencyChange, float amplitudeChange);
};