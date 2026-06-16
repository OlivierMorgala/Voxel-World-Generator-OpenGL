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

// Konstruktor klasy PerlinNoise2D inicjalizuje podstawowe parametry
PerlinNoise2D::PerlinNoise2D(unsigned seed, float freq, float amp, int octav, float freqChange, float ampChange)
    : frequency(freq), amplitude(amp), octaves(octav), frequencyChange(freqChange), amplitudeChange(ampChange)
{
    setSeed(seed);
}

// Metoda evaluate: Glowna funkcja ktora wyznacza  ostateczny noise  
float PerlinNoise2D::evaluate(float x, float z) {
    float rawNoise = perlinNoiseFunction(x, z);

    // Zwracamy znormalizowana wartosc (0.0f do 1.0f)
    return (rawNoise + 1.0f) * 0.5f;
}


float PerlinNoise2D::evaluate3D(float x, float y, float z) {
    return evaluate(x, z);
}

/// Metoda integrujaca z ImGui
/// Metoda renderImGui: tworzy wygodne suwaki dzieku ktorym mozna wygodnie zmianiac parametry PerliNoise'a
void PerlinNoise2D::renderImGui() {
    ImGui::DragFloat("Frequency", &frequency, 0.001f, 0.0001f, 2.0f);
    ImGui::DragFloat("Amplitude", &amplitude, 0.5f, 0.0f, 1000.0f);
    ImGui::DragInt("Octaves", &octaves, 0.1f, 1, 8);
    ImGui::DragFloat("FrequencyChange", &frequencyChange, 0.01f, 0.01f, 5.0f);
    ImGui::DragFloat("AmplitudeChange", &amplitudeChange, 0.01f, 0.01f, 2.0f);
}


/// Metoda setSeed: przygotowania tablicy permutacji

void PerlinNoise2D::setSeed(int newSeed) {
    permutations.resize(256);

    // Wypelniamy tablice wartosciami od 0 do 255
    std::iota(permutations.begin(), permutations.end(), 0);

    // Na podstawie danego seeda tasujemy tablice
    std::default_random_engine engine(newSeed);
    std::shuffle(permutations.begin(), permutations.end(), engine);

    // Duplikujemy ta tablice -> unikamy w ten sposob problemow z mozliwoscia wyjscia za tablice i w ten sposob ulatwiamy sobie implementacja perlinoise'a
    permutations.insert(permutations.end(), permutations.begin(), permutations.end());
}

float PerlinNoise2D::smoothInterpolation(float t, float x, float y) {
    return x + t * t * t * (t * (6.0f * t - 15.0f) + 10.0f) * (y - x);
}

/// Metoda DotProduct: standardowy iloczyn skalarny (dotProduct) dwoch wektrow 2D
float PerlinNoise2D::dotProduct(float gx, float gy, float x, float y) {
    return gx * x + gy * y;
}

/// Metoda perlinNoiseFunction: Serce algorytmu oblicza zsumowanego Noise'a
float PerlinNoise2D::perlinNoiseFunction(float x, float y) {
    float totalNoiseValue = 0.0;
    float frequencyPerlin = frequency;
    float amplitudePerlin = amplitude;
	float maxAmplitude = 0.0f;

    for (int i = 0; i < octaves; i++) {

        // Skalowanie wspolrzednych wejsciowych przez aktualna czestotliwosc
        float fx = x * frequencyPerlin;
        float fy = y * frequencyPerlin;

        // Wyznaczenie wspolrzednych lewego dolnego rogu siatki
        int x0 = static_cast<int>(std::floor(fx));
        int y0 = static_cast<int>(std::floor(fy));
        
        // Wyznaczenie wspolrzednych prawego gornego rogu siatki
        int x1 = x0 + 1;
        int y1 = y0 + 1;

        // Wyznaczenie pozycji punktu wewnatrz bierzacego kwadratu
        float ix = fx - x0;
        float iy = fy - y0;

        int p00 = permutations[(permutations[x0 & 255] + y0) & 255];
        int p10 = permutations[(permutations[x1 & 255] + y0) & 255];
        int p01 = permutations[(permutations[x0 & 255] + y1) & 255];
        int p11 = permutations[(permutations[x1 & 255] + y1) & 255];


        // Przypisanie pseudo losowych gradientow dla kazdego wierzcholka -> & 7 ogranicza indeks do rozmiaru tablicy (0-7)
        vector2 g00 = gradients[p00 & 7];
        vector2 g10 = gradients[p10 & 7];
        vector2 g01 = gradients[p01 & 7];
        vector2 g11 = gradients[p11 & 7];

        // Obliczenie iloczynu skalarnego dla kazdego wierzcholka miedzy wektorem gradientu, a wektorem odleglosci od wierzcholkow do punktu (ix, iy)
        float dp00 = dotProduct(g00.x, g00.y, ix, iy);
        float dp10 = dotProduct(g10.x, g10.y, ix - 1, iy);
        float dp01 = dotProduct(g01.x, g01.y, ix, iy - 1);
        float dp11 = dotProduct(g11.x, g11.y, ix - 1, iy - 1);

        // Interpolacja wynikow -> najpierw gora siatki potem dol -> Laczymy wyniki z wierzcholkow (0,0) - (0,1), a potem (1,0) - (1,1). Czyli z 4 wartosci w wierzcholkach robia nam sie dwa
        float interpolationXUp = smoothInterpolation(ix, dp00, dp10);
        float interpolationXDown = smoothInterpolation(ix, dp01, dp11);

        // Interpolacja wynikow wzdluz osi Y -> Laczymy oba wyniki z osi X w jeden
        float interpolationValue = smoothInterpolation(iy, interpolationXUp, interpolationXDown);

        // Dodajemy do siebie wartosc Noise'a pomnozonej  przez amplitiude dla bierzacej oktawy
        totalNoiseValue += interpolationValue * amplitudePerlin;

        // Sumuje calkowita amplitiude
		maxAmplitude += amplitudePerlin;

        // Modyfikujemy parametry dla kolejnej oktawy
        frequencyPerlin *= frequencyChange;
        amplitudePerlin *= amplitudeChange;
    }

    // NORMALIZACJA: Zabezpiecza wynik w przedziale [-1, 1]
    return totalNoiseValue / maxAmplitude;
}