#include "world/generationAlgorithms/PerlinNoise2D.h"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>

// Definicja gradientów
static vector2 gradients[8] = {
    {0,1}, {1,1}, {1,0}, {0,-1}, {-1,-1}, {-1,0}, {1,-1}, {-1,1},
};

PerlinNoise2D::PerlinNoise2D(std::string name, int startY, int endY, unsigned seed,
    float frequency, float amplitude, int octaves,
    float freqchange, float ampchange)
    : GenerationAlgorithm(name, startY, endY), // Inicjalizacja klasy bazowej
    frequencyPerlinNoise(frequency), amplitudePerlinNoise(amplitude),
    octavesPerlinNoise(octaves), frequencyChange(freqchange), amplitudeChange(ampchange)
{
    PermutationPerlin.resize(256);
    std::iota(PermutationPerlin.begin(), PermutationPerlin.end(), 0);

    std::default_random_engine engine(seed);
    std::shuffle(PermutationPerlin.begin(), PermutationPerlin.end(), engine);

    PermutationPerlin.insert(PermutationPerlin.end(), PermutationPerlin.begin(), PermutationPerlin.end());
}

void PerlinNoise2D::applyToColumn() {

}

void PerlinNoise2D::renderImGuiSettings() {
    if (ImGui::TreeNode(layerName.c_str())) {
        ImGui::DragFloat("Czêstotliwoœæ", &frequencyPerlinNoise, 0.01f);
        ImGui::DragFloat("Amplituda", &amplitudePerlinNoise, 0.1f);
        ImGui::DragInt("Oktawy", &octavesPerlinNoise, 1, 1, 8);
        ImGui::DragFloat("Zmiana Freq", &frequencyChange, 0.01f);
        ImGui::DragFloat("Zmiana Amp", &amplitudeChange, 0.01f);
        ImGui::DragInt("Start Y", &startY);
        ImGui::DragInt("End Y", &endY);
        ImGui::TreePop();
    }
}

float PerlinNoise2D::smoothInterpolation(float t, float x, float y) {
    return x + t * t * t * (t * (6.0f * t - 15.0f) + 10.0f) * (y - x);
}

float PerlinNoise2D::DotProduct(float gx, float gy, float x, float y) {
    return gx * x + gy * y;
}

float PerlinNoise2D::PerlinNoiseFunction(float x, float y) {
    float TotalNoiseValue = 0.0;
    float frequency = frequencyPerlinNoise;
    float amplitude = amplitudePerlinNoise;

    for (int i = 0; i < octavesPerlinNoise; i++) {
        float fx = x * frequency;
        float fy = y * frequency;

        int x0 = static_cast<int>(std::floor(fx));
        int y0 = static_cast<int>(std::floor(fy));
        int x1 = x0 + 1;
        int y1 = y0 + 1;

        float ix = fx - x0;
        float iy = fy - y0;

        int p00 = PermutationPerlin[(PermutationPerlin[x0 & 255] + y0) & 255];
        int p10 = PermutationPerlin[(PermutationPerlin[x1 & 255] + y0) & 255];
        int p01 = PermutationPerlin[(PermutationPerlin[x0 & 255] + y1) & 255];
        int p11 = PermutationPerlin[(PermutationPerlin[x1 & 255] + y1) & 255];

        vector2 g00 = gradients[p00 & 7];
        vector2 g10 = gradients[p10 & 7];
        vector2 g01 = gradients[p01 & 7];
        vector2 g11 = gradients[p11 & 7];

        float dp00 = DotProduct(g00.x, g00.y, ix, iy);
        float dp10 = DotProduct(g10.x, g10.y, ix - 1, iy);
        float dp01 = DotProduct(g01.x, g01.y, ix, iy - 1);
        float dp11 = DotProduct(g11.x, g11.y, ix - 1, iy - 1);

        float interpolationXUp = smoothInterpolation(ix, dp00, dp10);
        float interpolationXDown = smoothInterpolation(ix, dp01, dp11);
        float interpolationValue = smoothInterpolation(iy, interpolationXUp, interpolationXDown);

        TotalNoiseValue += interpolationValue * amplitude;

        frequency *= frequencyChange;
        amplitude *= amplitudeChange;
    }

    return TotalNoiseValue;
}