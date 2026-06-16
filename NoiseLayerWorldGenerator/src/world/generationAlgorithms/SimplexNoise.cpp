#include "world/generationAlgorithms/SimplexNoise.h"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>
#include <imgui.h>

// Zdefiniowanie Gradientow
static vector2 GradientsSimplex[12] = {
    {1,1}, {-1,1}, {1,-1}, { -1, -1},
    {1,0}, {-1,0}, {1,0}, {-1,0},
    {0,1}, {0,-1}, {0,1}, {0,-1}
};

// Konstruktor klasy SimplexNoise -> inicjalizuje podstawowe zmienne
SimplexNoise::SimplexNoise(unsigned seed, float freq, float amp, int octav, float freqchange, float ampchange)
    : frequency(freq), amplitude(amp), octaves(octav), frequencyChange(freqchange), amplitudeChange(ampchange)
{
    setSeed(seed);
}

// Metoda evaluate: Metoda odpowiedzialna za laczenie kilku warstw szumu (oktawy) w jeden zlozony noise
float SimplexNoise::evaluate(float x, float z)
{
    float totalNoise = 0.0f;
    float currentFrequency = frequency;
    float currentAmplitude = amplitude;
    float maxAmplitude = 0.0f;

    // Petla nakladania na siebie nowych oktaw
    for (int oct = 0; oct < octaves; oct++) {
        // Obliczam szum dla tej warstwy korzystajac z glownej metody odpowiadajacej za dzialanie alorytmu
        float noiseContribution = simplexNoiseFunction(x * currentFrequency, z * currentFrequency) * currentAmplitude;

        // Do calosci Noise'a dodaje wplyw aktualnej oktawy / i 
        totalNoise += noiseContribution;
        maxAmplitude += currentAmplitude; 

        // Modyfikuje paramtry dla nastepnej oktawy
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


// Metoda renderImGui: Metoda odpowiedzialna za stworzenie suwakow do modyfikowania parametrow przy uzyciu ImGui
void SimplexNoise::renderImGui()
{
    ImGui::DragFloat("Frequency", &frequency, 0.001f, 0.0001f, 2.0f, "%.4f");
    ImGui::DragFloat("Amplitude", &amplitude, 0.5f, 0.0f, 1000.0f, "%.1f");
    ImGui::DragInt("Octaves", &octaves, 0.1f, 1, 8);
    ImGui::DragFloat("FrequencyChange", &frequencyChange, 0.01f, 0.01f, 5.0f, "%.3f");
    ImGui::DragFloat("AmplitudeChange", &amplitudeChange, 0.01f, 0.01f, 2.0f, "%.3f");
}
// Metoda setSeed: przygotowania tablicy permutacji
void SimplexNoise::setSeed(int newSeed) {
    PermutationSimplex.clear();
    PermutationSimplex.resize(256);
    // Wypelniamy tablice wartosciami od 0 do 255
    std::iota(PermutationSimplex.begin(), PermutationSimplex.end(), 0);

    // Na podstawie seeda tasujemy nasza tabele permutacji
    std::default_random_engine engine(newSeed);
    std::shuffle(PermutationSimplex.begin(), PermutationSimplex.end(), engine);

    // Powielamy ta tablice -> unikamy w ten sposob problemow z mozliwoscia wyjscia za tablice i w ten sposob ulatwiamy sobie implementacja perlinoise'a
    PermutationSimplex.insert(PermutationSimplex.end(), PermutationSimplex.begin(), PermutationSimplex.end());
}

/// Metoda hash: Metoda mieszajaca (Hash) - wyciaga z tablicy unikalna liczbe dla danych wspolrzednych
int SimplexNoise::hash(int x, int y)
{
    return PermutationSimplex[PermutationSimplex[x & 255] + (y & 255)];
}

/// Metoda DotProduct: standardowy iloczyn skalarny (dotProduct) dwoch wektrow 2D 
float SimplexNoise::dotProduct(float gx, float gy, float x, float y)
{
    return gx * x + gy * y;
}

/// Metoda simplexNoiseFunction: Serce algorytmu, oblicza wartosc noise'a dla konkretnych wspolrzednych
float SimplexNoise::simplexNoiseFunction(float x, float y)
{
    //Zamiast kwadratowych klocków(jak w Perlinie), u¿ywa siatki trójk¹tów równobocznych.

    float Noise1 = 0.0f, Noise2 = 0.0f, Noise3 = 0.0f;

    /// STALE UZYWANE W SKEWING I UNSKEWING 
    // F2: Pochyla zwykla kwadratowa przestrzen w romby
    // G2: Prostuje romby z powrotem w idealne trojkaty rownoboczne
    float F2 = (sqrt(3.0f) - 1.0f) / 2.0f;
    float G2 = (3.0f - sqrt(3.0f)) / 6.0f;

    // Pochylamy przestrzen by dowiedziec sie w ktorym trojkacie rownobocznym (i,j) wyladowal nasz punkt
    float skew = (x + y) * F2;
    int i = std::floor(x + skew);
    int j = std::floor(y + skew);

    //  Prostujemy wspolrzedne wierzcholka 0, do normalnego swiata 2D
    float unskew = (i + j) * G2;
    float X0 = i - unskew; 
    float Y0 = j - unskew;

    // Wektor odleglosci punktu (x,y) od wierzcholka 0 
    float x0 = x - X0; 
    float y0 = y - Y0;

    /// Kwadrat zostal podzielony na dwa trojkaty dlatego na podstawie wspolrzednych sprawdzamy w ktorym dolnym czy gornym trojkacie znajduje sie nasz punkt
    int i1, j1;
    if (x0 > y0) {
        i1 = 1; // Dolne trojkat
        j1 = 0;
    }
    else {
        i1 = 0; // Gorny trojkat
        j1 = 1;
    }

    // Obliczenie odleglosci od pozostalych dwoch wierzcholkach trojkata (wierzcholku 1 i 2)
    float x1 = x0 - i1 + G2;
    float y1 = y0 - j1 + G2;
    float x2 = x0 - 1.0f + 2.0f * G2;
    float y2 = y0 - 1.0f + 2.0f * G2;

    // Losujemy wektory gradientu (kierunkowe) dla kazdego z trzech rogow trojkata
    vector2 g0 = GradientsSimplex[hash(i, j) % 12];
    vector2 g1 = GradientsSimplex[hash(i + i1, j + j1) % 12];
    vector2 g2 = GradientsSimplex[hash(i + 1, j + 1) % 12];

    // Wierzcholek 0 -> obliczamy jak mocno wplywa on na ostateczny noise
    // t mozna nazwac zasiegiem wplywu zapis - x*x - y*y to po prostu niespierwastkowane twierdzenie pitagorasa
    // czym wieksze t, tym bardziej ten wierzcholek wplywa na noise 
    float t0 = 0.5f - x0 * x0 - y0 * y0; 

    // Jesli t jest wieksze od 0 to wplywa na ten wierzcholek
    if (t0 > 0.0f) {
        t0 *= t0;
        Noise1 = t0 * t0 * dotProduct(g0.x, g0.y, x0, y0);
    }

    // Wierzcholek 1 -> obliczamy jak mocno wplywa on na ostateczny noise
    float t1 = 0.5f - x1 * x1 - y1 * y1;
    if (t1 > 0.0f) {
        t1 *= t1;
        Noise2 = t1 * t1 * dotProduct(g1.x, g1.y, x1, y1);
    }

    // Wierzcholek 2 -> obliczamy jak mocno wplywa on na ostateczny noise
    float t2 = 0.5f - x2 * x2 - y2 * y2;
    if (t2 > 0.0f) {
        t2 *= t2;
        Noise3 = t2 * t2 * dotProduct(g2.x, g2.y, x2, y2);
    }
    // Mnozymy wynik przez 43.0f -> to jest stala normalizacji dzieki ktorej wynik bedzie w przedziale [-1,1]
    return 43.0f * (Noise1 + Noise2 + Noise3); 
}