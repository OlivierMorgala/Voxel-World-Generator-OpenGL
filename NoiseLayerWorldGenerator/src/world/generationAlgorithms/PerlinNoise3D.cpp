#include "world/generationAlgorithms/PerlinNoise3D.h"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>
#include <imgui.h>

// Definicja gradientow
static vector3 gradients3D[12] = {
    {1,1,0}, {-1,1,0}, {1,-1,0}, {-1,-1,0},
    {1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
    {0,1,1}, {0,-1,1}, {0,1,-1}, {0,-1,-1}
};

PerlinNoise3D::PerlinNoise3D(unsigned seed, float freq, float amp, int octav, float freqChange, float ampChange)
    : frequency(freq), amplitude(amp), octaves(octav), frequencyChange(freqChange), amplitudeChange(ampChange)
{
    setSeed(seed);
}

float PerlinNoise3D::evaluate(float x, float z) {
    // W kontekscie mapy 2D, pobieramy warstwe szumu 3D dla y = 0.0f
    float rawNoise = perlin3DNoiseFunction(x, 0.0f, z);

    // Zwracamy znormalizowana wartosc (0.0f do 1.0f)
    return (rawNoise + 1.0f) * 0.5f;
}

float PerlinNoise3D::evaluate3D(float x, float y, float z) {
    float rawNoise = perlin3DNoiseFunction(x, y, z);
    return (rawNoise + 1.0f) * 0.5f;
}

void PerlinNoise3D::renderImGui() {
    ImGui::DragFloat("Frequency", &frequency, 0.001f, 0.0001f, 2.0f);
    ImGui::DragFloat("Amplitude", &amplitude, 0.5f, 0.0f, 1000.0f);
    ImGui::DragInt("Octaves", &octaves, 0.1f, 1, 8);
    ImGui::DragFloat("FrequencyChange", &frequencyChange, 0.01f, 0.01f, 5.0f);
    ImGui::DragFloat("AmplitudeChange", &amplitudeChange, 0.01f, 0.01f, 2.0f);
}

void PerlinNoise3D::setSeed(int newSeed) {
    PermutationPerlin3D.clear();
    PermutationPerlin3D.resize(256);
    std::iota(PermutationPerlin3D.begin(), PermutationPerlin3D.end(), 0);

    std::default_random_engine engine(newSeed);
    std::shuffle(PermutationPerlin3D.begin(), PermutationPerlin3D.end(), engine);

    PermutationPerlin3D.insert(PermutationPerlin3D.end(), PermutationPerlin3D.begin(), PermutationPerlin3D.end());
}

float PerlinNoise3D::smoothInterpolation(float t, float x, float y) {
    return x + t * t * t * (t * (6.0f * t - 15.0f) + 10.0f) * (y - x);
}

float PerlinNoise3D::dotProduct(float gx, float gy, float gz, float x, float y, float z) {
    return gx * x + gy * y + gz * z;
}

int PerlinNoise3D::hash(int x, int y, int z) {
    return PermutationPerlin3D[(PermutationPerlin3D[(PermutationPerlin3D[x & 255] + y) & 255] + z) & 255];
}

float PerlinNoise3D::perlin3DNoiseFunction(float x, float y, float z) {
    float TotalNoiseValue = 0.0f;
    float currentFrequency = frequency;
    float currentAmplitude = amplitude;
    float maxAmplitude = 0.0f;

    for (int i = 0; i < octaves; i++) {
        float fx = x * currentFrequency;
        float fy = y * currentFrequency;
        float fz = z * currentFrequency;

        int x0 = static_cast<int>(std::floor(fx));
        int y0 = static_cast<int>(std::floor(fy));
        int z0 = static_cast<int>(std::floor(fz));

        int x1 = x0 + 1;
        int y1 = y0 + 1;
        int z1 = z0 + 1;

        float ix = fx - x0;
        float iy = fy - y0;
        float iz = fz - z0;

        int p000 = hash(x0, y0, z0);
        int p100 = hash(x1, y0, z0);
        int p110 = hash(x1, y1, z0);
        int p111 = hash(x1, y1, z1);
        int p001 = hash(x0, y0, z1);
        int p011 = hash(x0, y1, z1);
        int p010 = hash(x0, y1, z0);
        int p101 = hash(x1, y0, z1);

        vector3 g000 = gradients3D[p000 % 12];
        vector3 g100 = gradients3D[p100 % 12];
        vector3 g110 = gradients3D[p110 % 12];
        vector3 g111 = gradients3D[p111 % 12];
        vector3 g001 = gradients3D[p001 % 12];
        vector3 g011 = gradients3D[p011 % 12];
        vector3 g010 = gradients3D[p010 % 12];
        vector3 g101 = gradients3D[p101 % 12];

        //// DOLNA PODSTAWA SZESCIANU (ILOCZYN SKALARNY)
        float dp000 = dotProduct(g000.x, g000.y, g000.z, ix, iy, iz);
        float dp100 = dotProduct(g100.x, g100.y, g100.z, ix - 1, iy, iz);
        float dp001 = dotProduct(g001.x, g001.y, g001.z, ix, iy, iz - 1);
        float dp101 = dotProduct(g101.x, g101.y, g101.z, ix - 1, iy, iz - 1);

        /// GORNA PODSTAWA SZESCIANU (ILOCZYN SKALARNY)
        float dp010 = dotProduct(g010.x, g010.y, g010.z, ix, iy - 1, iz);
        float dp110 = dotProduct(g110.x, g110.y, g110.z, ix - 1, iy - 1, iz);
        float dp011 = dotProduct(g011.x, g011.y, g011.z, ix, iy - 1, iz - 1);
        float dp111 = dotProduct(g111.x, g111.y, g111.z, ix - 1, iy - 1, iz - 1);

        //// SZESCIAN MA 8 WIERZCHOLKOW WIEC LACZYMY WARTOSCI ZE WSZYSTKICH W JEDNA KONCOWA WARTOSC NOISE
        // NA POCZATU INTERPOLUJEMY PRZEZ X

        /// KROK 1 -> LACZYMY LEWE I PRAWE NAROZNIKI NA OSI X -> NA KAZDEJ KRAWEDZI SPRAWDZAMY WARTOSC UZYSKANEGO NOISE W PUNKCIE IX. ZAMIAST 8 WIERZCHOLKOW MAMY TERAZ 4 PUNKTY NA KAZDEJ KRAWEDZI NA OSI X.
        float InterpolationX00 = smoothInterpolation(ix, dp000, dp100); // LACZY WARTOSC (0,0,0) z (1,0,0)
        float InterpolationX10 = smoothInterpolation(ix, dp010, dp110); // LACZY WARTOSC (0,1,0) z (1,1,0)
        float InterpolationX01 = smoothInterpolation(ix, dp001, dp101); // LACZY WARTOSC (0,0,1) z (1,0,1)
        float InterpolationX11 = smoothInterpolation(ix, dp011, dp111); // LACZY WARTOSC (0,1,1) z (1,1,1)

        /// KROK 2 -> MAMY TERAZ 4 PUNKTY, PO JEDNEJ NA KAZDEJ Z GORNYCH KRAWEDZI I PO JEDNEJ NA KAZDEJ Z DOLNYCH. TERAZ LACZYMY DLA PRZEDNIEJ I TYLNEJ SCIANY TE PUNKTY. Z 4 PUNKTOW TWORZA NAM SIE DWA.
        float InterpolationY0 = smoothInterpolation(iy, InterpolationX00, InterpolationX10);
        float InterpolationY1 = smoothInterpolation(iy, InterpolationX01, InterpolationX11);

        /// KROK 3 -> TERAZ MAMY DWA PUNKTY I LACZYMY JE W OSI Z, TERAZ ZOSTAJE NAM JEDEN PUNKT GDZIES W SRODKU SZESCIANU
        float InterpolationFinalZ = smoothInterpolation(iz, InterpolationY0, InterpolationY1);

        TotalNoiseValue += InterpolationFinalZ * currentAmplitude;
        maxAmplitude += currentAmplitude;

        currentFrequency *= frequencyChange;
        currentAmplitude *= amplitudeChange;
    }

    // NORMALIZACJA: Zabezpiecza wynik w przedziale [-1, 1]
    return TotalNoiseValue / maxAmplitude;
}