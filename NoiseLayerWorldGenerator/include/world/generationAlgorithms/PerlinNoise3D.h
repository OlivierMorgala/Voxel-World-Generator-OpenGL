#pragma once
#include <vector>
#include <string>
#include "world/TerrainPipeline.h"

struct vector3 {
    float x, y, z;
};

class PerlinNoise3D : public TerrainAlgorithm
{
private:
    float frequency;
    float amplitude;
    int octaves;
    float frequencyChange; 
    float amplitudeChange; 
    std::vector<int> PermutationPerlin3D;

    // Funkcje pomocnicze
    float perlin3DNoiseFunction(float x, float y, float z);
    float dotProduct(float gx, float gy, float gz, float x, float y, float z);
    float smoothInterpolation(float t, float x, float y);
    int hash(int x, int y, int z);

public:
    PerlinNoise3D(unsigned seed = 1234, float frequency = 1.0f, float amplitude = 1.0f,
        int octaves = 4, float freqchange = 2.0f, float ampchange = 0.5f);

    
    float evaluate(float x, float z) override;
    float evaluate3D(float x, float y, float z) override;
    void renderImGui() override;
    void setSeed(int newSeed) override;
};