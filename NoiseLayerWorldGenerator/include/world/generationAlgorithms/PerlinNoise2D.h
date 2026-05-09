#pragma once
#include <vector>
#include <string>
#include "world/generationAlgorithms/GenerationAlgorithm.h"

struct vector2 {
    float x, y;
};

class PerlinNoise2D : public GenerationAlgorithm
{
public:
    float frequencyPerlinNoise; //
    float amplitudePerlinNoise; //
    int octavesPerlinNoise; //
    float frequencyChange; // Decyduje jak zmienia sie frequency po kazdej oktawie
    float amplitudeChange; // Decyduje jak zmienia sie amplitude po kazdej oktawie

    PerlinNoise2D(std::string name, int startY, int endY, unsigned seed = 1234,
        float frequency = 1.0f, float amplitude = 1.0f,
        int octaves = 4, float freqchange = 2.0f, float ampchange = 0.5f);

    // Implementacja metod wirtualnych
    virtual void applyToColumn() override;
    virtual void renderImGuiSettings() override;

    // Funkcje pomocnicze
    float PerlinNoiseFunction(float x, float y);
    float DotProduct(float gx, float gy, float x, float y);
    float smoothInterpolation(float t, float x, float y);

private:
    std::vector<int> PermutationPerlin;
};