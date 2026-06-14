#include "world/generationAlgorithms/PerlinNoise2D.h"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>

struct vector2 {
    float x;
    float y;
};

// Definicja gradientow
static vector2 gradients[8] = {
    {0,1}, {1,1}, {1,0}, {0,-1}, {-1,-1}, {-1,0}, {1,-1}, {-1,1},
};

PerlinNoise2D::PerlinNoise2D(unsigned seed, float freq, float amp, int octav, float freqChange, float ampChange)
    : frequency(freq), amplitude(amp), octaves(octav), frequencyChange(freqChange), amplitudeChange(ampChange)
{
    setSeed(seed);
}

float PerlinNoise2D::evaluate(float x, float z) {
    float rawNoise = perlinNoiseFunction(x, z);

    return (rawNoise + 1.0f) * 0.5f;
}

void PerlinNoise2D::renderImGui() {
    ImGui::DragFloat("Frequency", &frequency, 0.01f);
    ImGui::DragFloat("Amplitude", &amplitude, 0.1f);
    ImGui::DragInt("Octaves", &octaves, 1, 1, 8);
    ImGui::DragFloat("FrequencyChange", &frequencyChange, 0.01f);
    ImGui::DragFloat("AmplitudeChange", &amplitudeChange, 0.01f);
}

void PerlinNoise2D::setSeed(int newSeed) {
    permutations.resize(256);

    std::iota(permutations.begin(), permutations.end(), 0);
    std::default_random_engine engine(newSeed);

    std::shuffle(permutations.begin(), permutations.end(), engine);

    permutations.insert(permutations.end(), permutations.begin(), permutations.end());
}

float PerlinNoise2D::smoothInterpolation(float t, float x, float y) {
    return x + t * t * t * (t * (6.0f * t - 15.0f) + 10.0f) * (y - x);
}

float PerlinNoise2D::dotProduct(float gx, float gy, float x, float y) {
    return gx * x + gy * y;
}

float PerlinNoise2D::perlinNoiseFunction(float x, float y) {
    float totalNoiseValue = 0.0;
    float frequencyPerlin = frequency;
    float amplitudePerlin = amplitude;
	float maxAmplitude = 0.0f;

    for (int i = 0; i < octaves; i++) {
        float fx = x * frequencyPerlin;
        float fy = y * frequencyPerlin;

        int x0 = static_cast<int>(std::floor(fx));
        int y0 = static_cast<int>(std::floor(fy));
        int x1 = x0 + 1;
        int y1 = y0 + 1;

        float ix = fx - x0;
        float iy = fy - y0;

        int p00 = permutations[(permutations[x0 & 255] + y0) & 255];
        int p10 = permutations[(permutations[x1 & 255] + y0) & 255];
        int p01 = permutations[(permutations[x0 & 255] + y1) & 255];
        int p11 = permutations[(permutations[x1 & 255] + y1) & 255];

        vector2 g00 = gradients[p00 & 7];
        vector2 g10 = gradients[p10 & 7];
        vector2 g01 = gradients[p01 & 7];
        vector2 g11 = gradients[p11 & 7];

        float dp00 = dotProduct(g00.x, g00.y, ix, iy);
        float dp10 = dotProduct(g10.x, g10.y, ix - 1, iy);
        float dp01 = dotProduct(g01.x, g01.y, ix, iy - 1);
        float dp11 = dotProduct(g11.x, g11.y, ix - 1, iy - 1);

        float interpolationXUp = smoothInterpolation(ix, dp00, dp10);
        float interpolationXDown = smoothInterpolation(ix, dp01, dp11);
        float interpolationValue = smoothInterpolation(iy, interpolationXUp, interpolationXDown);

        totalNoiseValue += interpolationValue * amplitudePerlin;

		maxAmplitude += amplitudePerlin;

        frequencyPerlin *= frequencyChange;
        amplitudePerlin *= amplitudeChange;
    }

    return totalNoiseValue / maxAmplitude;
}