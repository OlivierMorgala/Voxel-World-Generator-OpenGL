#pragma once
#include <vector>
#include <string>
#include "world/generationAlgorithms/GenerationAlgorithm.h"

struct vector3 
{ float x, y, z; };

class PerlinNoise3D : public GenerationAlgorithm
{
public:
    float frequency3DPerlinNoise; //
    float amplitude3DPerlinNoise; //
    int octaves3DPerlinNoise; //
    float frequencyChange; // Decyduje jak zmienia sie frequency po kazdej oktawie
    float amplitudeChange; // Decyduje jak zmienia sie amplitude po kazdej oktawie

    PerlinNoise3D(std::string name, int startY, int endY, unsigned seed = 1234,
        float frequency = 1.0f, float amplitude = 1.0f,
        int octaves = 4, float freqchange = 2.0f, float ampchange = 0.5f);

    // Implementacja metod wirtualnych
    virtual void applyToColumn(ChunkColumn& column) override;
    virtual void renderImGuiSettings() override;

    // Funkcje pomocnicze
    float Perlin3DNoiseFunction(float x, float y, float z);
    float DotProduct(float gx, float gy, float gz, float x, float y, float z);
    float smoothInterpolation(float t, float x, float y);

    int Hash(int x, int y, int z);
private:
    std::vector<int> PermutationPerlin3D;
};