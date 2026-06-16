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


/// Konstruktor klasy PerlinNoise3D -> inicjalizacja bazowych parametrow
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

/// Metoda integrujaca z ImGui
/// Metoda renderImGui: tworzy wygodne suwaki dzieku ktorym mozna wygodnie zmianiac parametry 3D PerliNoise'a
void PerlinNoise3D::renderImGui() {
    ImGui::DragFloat("Frequency", &frequency, 0.001f, 0.0001f, 2.0f);
    ImGui::DragFloat("Amplitude", &amplitude, 0.5f, 0.0f, 1000.0f);
    ImGui::DragInt("Octaves", &octaves, 0.1f, 1, 8);
    ImGui::DragFloat("FrequencyChange", &frequencyChange, 0.01f, 0.01f, 5.0f);
    ImGui::DragFloat("AmplitudeChange", &amplitudeChange, 0.01f, 0.01f, 2.0f);
}

// Metoda setSeed: przygotowania tablicy permutacji
void PerlinNoise3D::setSeed(int newSeed) {
    PermutationPerlin3D.clear();
    PermutationPerlin3D.resize(256);

    // Wypelniamy tablice wartosciami od 0 do 255
    std::iota(PermutationPerlin3D.begin(), PermutationPerlin3D.end(), 0);

    // Na podstawie danego seeda tasujemy tablice
    std::default_random_engine engine(newSeed);
    std::shuffle(PermutationPerlin3D.begin(), PermutationPerlin3D.end(), engine);

    // Duplikujemy ta tablice -> unikamy w ten sposob problemow z mozliwoscia wyjscia za tablice i w ten sposob ulatwiamy sobie implementacja perlinoise'a
    PermutationPerlin3D.insert(PermutationPerlin3D.end(), PermutationPerlin3D.begin(), PermutationPerlin3D.end());
}

float PerlinNoise3D::smoothInterpolation(float t, float x, float y) {
    return x + t * t * t * (t * (6.0f * t - 15.0f) + 10.0f) * (y - x);
}

/// Metoda DotProduct: standardowy iloczyn skalarny (dotProduct) dwoch wektrow 3D 
float PerlinNoise3D::dotProduct(float gx, float gy, float gz, float x, float y, float z) {
    return gx * x + gy * y + gz * z;
}

/// Metoda hash: Wykorzystuje potrojne mieszanie tablicy, zamienia operacje % na o wiele szybsze &
int PerlinNoise3D::hash(int x, int y, int z) {
    return PermutationPerlin3D[(PermutationPerlin3D[(PermutationPerlin3D[x & 255] + y) & 255] + z) & 255];
}

// Metoda perlin3DNoiseFunction: Serce algorytmu oblicza zsumowanego noise'a
float PerlinNoise3D::perlin3DNoiseFunction(float x, float y, float z) {
    float TotalNoiseValue = 0.0f;
    float currentFrequency = frequency;
    float currentAmplitude = amplitude;
    float maxAmplitude = 0.0f;

    for (int i = 0; i < octaves; i++) {

        // Skalowanie wspolrzednych wejsciowych przez aktualna czestotliwosc
        float fx = x * currentFrequency;
        float fy = y * currentFrequency;
        float fz = z * currentFrequency;

        // Wyznaczenie wspolrzednych lewego, dolnego, przedniego rogu szescianu
        int x0 = static_cast<int>(std::floor(fx));
        int y0 = static_cast<int>(std::floor(fy));
        int z0 = static_cast<int>(std::floor(fz));

        // Wyznaczenie wspolrzednych prawego, gornego, tylnego rogu siatki
        int x1 = x0 + 1;
        int y1 = y0 + 1;
        int z1 = z0 + 1;

        // Wyznaczenie pozycji punktu wewnatrz bierzacego szescianu
        float ix = fx - x0;
        float iy = fy - y0;
        float iz = fz - z0;

        // Przy pomocy funkcje hash kazdy wierzcholek uzyskuje wartosc od 0-255 wartosci te pozniej posluza do wyboru gradientu dla kazdego wierzcholka
        int p000 = hash(x0, y0, z0);
        int p100 = hash(x1, y0, z0);
        int p110 = hash(x1, y1, z0);
        int p111 = hash(x1, y1, z1);
        int p001 = hash(x0, y0, z1);
        int p011 = hash(x0, y1, z1);
        int p010 = hash(x0, y1, z0);
        int p101 = hash(x1, y0, z1);

        // Przypisanie pseudo losowego gradientu dla kazdego wierzcholka szescianu -> % 12 ogranicza indeksy do rozmiaru tablicy (0-11) -> w odroznieniu od perlinnoise 2d nie uzywamy & gdyz nie bedzie to dzialalo poprawnie dla 12
        vector3 g000 = gradients3D[p000 % 12];
        vector3 g100 = gradients3D[p100 % 12];
        vector3 g110 = gradients3D[p110 % 12];
        vector3 g111 = gradients3D[p111 % 12];
        vector3 g001 = gradients3D[p001 % 12];
        vector3 g011 = gradients3D[p011 % 12];
        vector3 g010 = gradients3D[p010 % 12];
        vector3 g101 = gradients3D[p101 % 12];

        //// DOLNA PODSTAWA SZESCIANU: Obliczanie iloczynu skalarnego miedzy wektorem gradientu, a wektorem odlegosci wierzcholka od punktu (ix,iy,iz)
        float dp000 = dotProduct(g000.x, g000.y, g000.z, ix, iy, iz);
        float dp100 = dotProduct(g100.x, g100.y, g100.z, ix - 1, iy, iz);
        float dp001 = dotProduct(g001.x, g001.y, g001.z, ix, iy, iz - 1);
        float dp101 = dotProduct(g101.x, g101.y, g101.z, ix - 1, iy, iz - 1);

        /// GORNA PODSTAWA SZESCIANU: Obliczanie iloczynu skalarnego miedzy wektorem gradientu, a wektorem odlegosci wierzcholka od punktu (ix,iy,iz)
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

        /// KROK 2 -> MAMY TERAZ 4 PUNKTY, PO JEDNEJ NA KAZDEJ Z GORNYCH KRAWEDZI X I PO JEDNEJ NA KAZDEJ Z DOLNYCH X. TERAZ LACZYMY DLA PRZEDNIEJ I TYLNEJ SCIANY TE PUNKTY. Z 4 PUNKTOW TWORZA NAM SIE DWA.
        float InterpolationY0 = smoothInterpolation(iy, InterpolationX00, InterpolationX10);
        float InterpolationY1 = smoothInterpolation(iy, InterpolationX01, InterpolationX11);

        /// KROK 3 -> TERAZ MAMY DWA PUNKTY NA DWOCH SCIANACH I LACZYMY JE W OSI Z, TERAZ ZOSTAJE NAM JEDEN PUNKT KTORY ZNAJDUJE SIE  W SRODKU SZESCIANU
        float InterpolationFinalZ = smoothInterpolation(iz, InterpolationY0, InterpolationY1);

        // Dodajemy do siebie wartosc Noise'a pomnozonej  przez amplitiude dla bierzacej oktawy
        TotalNoiseValue += InterpolationFinalZ * currentAmplitude;

        // Sumuje calkowita amplitiude
        maxAmplitude += currentAmplitude;

        // Modyfikujemy parametry dla kolejnej oktawy
        currentFrequency *= frequencyChange;
        currentAmplitude *= amplitudeChange;
    }

    // NORMALIZACJA: Zabezpiecza wynik w przedziale [-1, 1]
    return TotalNoiseValue / maxAmplitude;
}