#include "world/generationAlgorithms/SimplexNoise.h"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>
#include <imgui.h>

static vector2 GradientsSimplex[12] = {
    {1,1}, {-1,1}, {1,-1}, { -1, -1},
    {1,0}, {-1,0}, {1,0}, {-1,0},
    {0,1}, {0,-1}, {0,1}, {0,-1}
};

SimplexNoise::SimplexNoise(unsigned seed, float freq, float amp, int octav, float freqchange, float ampchange)
    : frequency(freq), amplitude(amp), octaves(octav), frequencyChange(freqchange), amplitudeChange(ampchange)
{
    setSeed(seed);
}

float SimplexNoise::evaluate(float x, float z)
{
    float totalNoise = 0.0f;
    float currentFrequency = frequency;
    float currentAmplitude = amplitude;
    float maxAmplitude = 0.0f;

    for (int oct = 0; oct < octaves; oct++) {
        float noiseContribution = simplexNoiseFunction(x * currentFrequency, z * currentFrequency) * currentAmplitude;

        totalNoise += noiseContribution;
        maxAmplitude += currentAmplitude;

        currentFrequency *= frequencyChange;
        currentAmplitude *= amplitudeChange;
    }

    // Normalizacja do przedzialu [-1.0f, 1.0f], a nastepnie przekonwertowanie na [0.0f, 1.0f]
    float normalizedRawNoise = totalNoise / maxAmplitude;
    return (normalizedRawNoise + 1.0f) * 0.5f;
}

float SimplexNoise::evaluate3D(float x, float y, float z) {
    return evaluate(x, z);
}

void SimplexNoise::renderImGui()
{
    ImGui::DragFloat("Frequency", &frequency, 0.001f, 0.0001f, 2.0f, "%.4f");
    ImGui::DragFloat("Amplitude", &amplitude, 0.5f, 0.0f, 1000.0f, "%.1f");
    ImGui::DragInt("Octaves", &octaves, 0.1f, 1, 8);
    ImGui::DragFloat("FrequencyChange", &frequencyChange, 0.01f, 0.01f, 5.0f, "%.3f");
    ImGui::DragFloat("AmplitudeChange", &amplitudeChange, 0.01f, 0.01f, 2.0f, "%.3f");
}

void SimplexNoise::setSeed(int newSeed) {
    PermutationSimplex.clear();
    PermutationSimplex.resize(256);
    std::iota(PermutationSimplex.begin(), PermutationSimplex.end(), 0);

    // NA PODSTAWIE SEEDA TASUJEMY NASZA TABELE PERMUTACJI 
    std::default_random_engine engine(newSeed);
    std::shuffle(PermutationSimplex.begin(), PermutationSimplex.end(), engine);

    // POWIELANIE TABLIY PERMUTACJI -> [0, ..., 255, 0, ..... 255], CHRONI PRZED WYJSCIEM POZA ROZMIAR WEKTORA
    PermutationSimplex.insert(PermutationSimplex.end(), PermutationSimplex.begin(), PermutationSimplex.end());
}

/// Funkcja Hashujaca
int SimplexNoise::hash(int x, int y)
{
    return PermutationSimplex[PermutationSimplex[x & 255] + (y & 255)];
}

/// FUNKCJA DOTPORUDCT ILOCZYN SKALARNY
float SimplexNoise::dotProduct(float gx, float gy, float x, float y)
{
    return gx * x + gy * y;
}

/// FUNKCJA NOISE'A 
float SimplexNoise::simplexNoiseFunction(float x, float y)
{
    //// Simplex Noise 2D JEST "ZBUDOWANY" Z TROJKATOW ROWNOBOCZONYCH -> ALGORYTM LICZY NOISE W KAZDYM Z WIERZCHOLKOW TROJKATA I NA KONIEC GO SUMUJE////
    float Noise1 = 0.0f, Noise2 = 0.0f, Noise3 = 0.0f;

    /// STALE UZYWANE W SKEWING I UNSKEWING 
    /// F2 TRANSFORMUJE KWADRATY NA ROMBY (POCHYLA PRZESTRZEN)
    /// G2 TRANSFORMUJE ROMBY W TROJKATY ROWNOBOCZNE
    float F2 = (sqrt(3.0f) - 1.0f) / 2.0f;
    float G2 = (3.0f - sqrt(3.0f)) / 6.0f;

    /// INFORMACJA W KTORYM KAFELKU (i,j) ZNAJDUJE SIE NASZ PUNKT
    float skew = (x + y) * F2;
    int i = std::floor(x + skew);
    int j = std::floor(y + skew);

    float unskew = (i + j) * G2;

    float X0 = i - unskew; // RZECZYWISTE WSPOLRZEDNE WIERZCHOLKA 0
    float Y0 = j - unskew;
    float x0 = x - X0; // WEKTOR ODLEGLOSCI PUNKTU (x,y) OD WIERZCHOLKA 0 
    float y0 = y - Y0;

    /// JESTLI x0>y0 TO PUNKT ZNAJDUJE SIE W DOLNYM TROJKACIE SIMPLEXA JESTLI y0 > x0 TO ZNAJDUJE SIE W GORNYM
    int i1, j1;
    if (x0 > y0) {
        i1 = 1;
        j1 = 0;
    }
    else {
        i1 = 0;
        j1 = 1;
    }

    // WEKTORY ODLEGLOSCI WIERZCHOLKA 1 / 2 OD PUNKTU (x,y)
    float x1 = x0 - i1 + G2;
    float y1 = y0 - j1 + G2;
    float x2 = x0 - 1.0f + 2.0f * G2;
    float y2 = y0 - 1.0f + 2.0f * G2;

    vector2 g0 = GradientsSimplex[hash(i, j) % 12];
    vector2 g1 = GradientsSimplex[hash(i + i1, j + j1) % 12];
    vector2 g2 = GradientsSimplex[hash(i + 1, j + 1) % 12];

    // OBLICZENIA JAK MOCNO WIERZCHOLEK NR 0 WPLYWA NA NOISE CALKOWITY
    float t0 = 0.5f - x0 * x0 - y0 * y0; // t0 to odleglosc
    if (t0 > 0.0f) {
        //  t0*=t0
        t0 *= t0;
        Noise1 = t0 * t0 * dotProduct(g0.x, g0.y, x0, y0);
    }

    // OBLICZENIA JAK MOCNO WIERZCHOLEK NR 1 WPLYWA NA NOISE CALKOWITY
    float t1 = 0.5f - x1 * x1 - y1 * y1;
    if (t1 > 0.0f) {
        t1 *= t1;
        Noise2 = t1 * t1 * dotProduct(g1.x, g1.y, x1, y1);
    }

    // OBLICZENIA JAK MOCNO WIERZCHOLEK NR 2 WPLYWA NA NOISE CALKOWITY
    float t2 = 0.5f - x2 * x2 - y2 * y2;
    if (t2 > 0.0f) {
        t2 *= t2;
        Noise3 = t2 * t2 * dotProduct(g2.x, g2.y, x2, y2);
    }

    return 43.0f * (Noise1 + Noise2 + Noise3); // FUNKCJA ZWRACA CALKOWITY NOISE (WSPOLCZYNNIK 43.0f TO STALA NORMALIZACJI MATEMATYCZNEJ DO PRZEDZIALU [-1.0,1.0]
}