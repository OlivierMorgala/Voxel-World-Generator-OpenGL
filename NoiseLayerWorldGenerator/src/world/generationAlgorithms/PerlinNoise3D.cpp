#include "world/generationAlgorithms/PerlinNoise3D.h"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>

// Definicja gradientow
static vector3 gradients[12] = {
    {1,1,0}, {-1,1,0}, {1,-1,0}, {-1,-1,0},
    {1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
    {0,1,1}, {0,-1,1}, {0,1,-1}, {0,-1,-1}
};

PerlinNoise3D::PerlinNoise3D(std::string name, int startY, int endY, unsigned seed,
    float frequency, float amplitude, int octaves,
    float freqchange, float ampchange)
    : GenerationAlgorithm(name, startY, endY), // Inicjalizacja klasy bazowej
    frequency3DPerlinNoise(frequency), amplitude3DPerlinNoise(amplitude),
    octaves3DPerlinNoise(octaves), frequencyChange(freqchange), amplitudeChange(ampchange)
{
    PermutationPerlin3D.resize(256);
    std::iota(PermutationPerlin3D.begin(), PermutationPerlin3D.end(), 0);

    std::default_random_engine engine(seed);
    std::shuffle(PermutationPerlin3D.begin(), PermutationPerlin3D.end(), engine);

    PermutationPerlin3D.insert(PermutationPerlin3D.end(), PermutationPerlin3D.begin(), PermutationPerlin3D.end());
}

void PerlinNoise3D::applyToColumn(ChunkColumn& column) {
    /*int worldXOffset = column.getX() * 16;
    int worldZOffset = column.getZ() * 16;
    
    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            float noise = Perlin3DNoiseFunction(worldXOffset + x, worldZOffset + z);

            int height = startY + static_cast<int>((noise + 1.0f) * 0.5f * (endY - startY));

            for (int y = 0; y <= height; y++) {
                column.setBlock(x, y, z, 1);
            }
        }
    }*/
}

void PerlinNoise3D::renderImGuiSettings() {
    if (ImGui::TreeNode(layerName.c_str())) {
        ImGui::DragFloat("Czestotliwosc", &frequency3DPerlinNoise, 0.01f);
        ImGui::DragFloat("Amplituda", &amplitude3DPerlinNoise, 0.1f);
        ImGui::DragInt("Oktawy", &octaves3DPerlinNoise, 1, 1, 8);
        ImGui::DragFloat("Zmiana Freq", &frequencyChange, 0.01f);
        ImGui::DragFloat("Zmiana Amp", &amplitudeChange, 0.01f);
        ImGui::DragInt("Start Y", &startY);
        ImGui::DragInt("End Y", &endY);
        ImGui::TreePop();
    }
}

float PerlinNoise3D::smoothInterpolation(float t, float x, float y) {
    return x + t * t * t * (t * (6.0f * t - 15.0f) + 10.0f) * (y - x);
}

float PerlinNoise3D::DotProduct(float gx, float gy, float gz, float x, float y, float z) {
    return gx * x + gy * y + gz * z;
}

int PerlinNoise3D::Hash(int x, int y, int z)
{
    return PermutationPerlin3D[(PermutationPerlin3D[(PermutationPerlin3D[x & 255] + y) & 255] + z) & 255];
}

float PerlinNoise3D::Perlin3DNoiseFunction(float x, float y, float z) {
    float TotalNoiseValue = 0.0;
    float frequency = frequency3DPerlinNoise;
    float amplitude = amplitude3DPerlinNoise;

    for (int i = 0; i < octaves3DPerlinNoise; i++) {
        float fx = x * frequency;
        float fy = y * frequency;
        float fz = z * frequency;
        int x0 = static_cast<int>(std::floor(fx));
        int y0 = static_cast<int>(std::floor(fy));
        int z0 = static_cast<int>(std::floor(fz));

        int x1 = x0 + 1;
        int y1 = y0 + 1;
        int z1 = z0 + 1;

        float ix = fx - x0;
        float iy = fy - y0;
        float iz = fz - z0;

        int p000 = Hash(x0, y0, z0);
        int p100 = Hash(x1, y0, z0);
        int p110 = Hash(x1, y1, z0);
        int p111 = Hash(x1, y1, z1);
        int p001 = Hash(x0, y0, z1);
        int p011 = Hash(x0, y1, z1);
        int p010 = Hash(x0, y1, z0);
        int p101 = Hash(x1, y0, z1);

        vector3 g000 = gradients[p000 % 12];
        vector3 g100 = gradients[p100 % 12];
        vector3 g110 = gradients[p110 % 12];
        vector3 g111 = gradients[p111 % 12];
        vector3 g001 = gradients[p001 % 12];
        vector3 g011 = gradients[p011 % 12];
        vector3 g010 = gradients[p010 % 12];
        vector3 g101 = gradients[p101 % 12];

        //// DOLNA PODSTAWA SZESCIANU (ILOCZYN SKALARNY)
        float dp000 = DotProduct(g000.x, g000.y, g000.z , ix, iy, iz);
        float dp100 = DotProduct(g100.x, g100.y, g100.z, ix - 1, iy, iz);
        float dp001 = DotProduct(g001.x, g001.y, g001.z, ix, iy, iz-1);
        float dp101 = DotProduct(g101.x, g101.y, g101.z, ix - 1, iy, iz - 1);

        /// GORNA PODSTAWA SZESCIANU (ILOCZYN SKALARNY)
        float dp010 = DotProduct(g010.x, g010.y, g010.z, ix, iy - 1, iz);
        float dp110 = DotProduct(g110.x, g110.y, g110.z, ix - 1, iy - 1, iz);
        float dp011 = DotProduct(g011.x, g011.y, g011.z, ix, iy - 1, iz - 1);
        float dp111 = DotProduct(g111.x, g111.y, g111.z, ix - 1, iy - 1, iz - 1);
    
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
        TotalNoiseValue += InterpolationFinalZ * amplitude;
        frequency *= frequencyChange;
        amplitude *= amplitudeChange;
    }

    return TotalNoiseValue;
}