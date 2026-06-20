#pragma once
#include <vector>
#include <string>
#include "world/TerrainPipeline.h"

struct vector2 {
    float x, y;
};

// Klasa SimplexNoise
class SimplexNoise : public TerrainAlgorithm
{
private:
    float frequency;
    float amplitude;
    int octaves;
    float frequencyChange;
    float amplitudeChange;
    std::vector<int> PermutationSimplex;

    // Funkcje pomocnicze
    float dotProduct(float gx, float gy, float x, float y);
    float simplexNoiseFunction(float x, float y);
    int hash(int x, int y);

public:
    SimplexNoise(unsigned seed = 1234, float freq = 1.0f, float amp = 1.0f,
        int octav = 4, float freqchange = 2.0f, float ampchange = 0.5f);

    // Implementacja metod wirtualnych
    float evaluate(float x, float z) override;
    float evaluate3D(float x, float y, float z) override;
    void renderImGui() override;
    void setSeed(int newSeed) override;
};