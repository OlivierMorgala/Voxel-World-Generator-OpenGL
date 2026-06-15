#include "world/generationAlgorithms/SimplexNoise.h"

vector2 Gradients[12] = {
	{1,1}, {-1,1}, {1,-1}, { -1, -1},
	{1,0}, {-1,0}, {1,0}, {-1,0},
	{0,1}, {0,-1}, {0,1}, {0,-1}
};


SimplexNoise::SimplexNoise(std::string Name, int startY, int endY, int seedSimpexNoise, float frequency, float amplitude, int octaves, float freqchange, float ampchange) :
	GenerationAlgorithm(Name, startY, endY),
	seedSimplexNoise(seedSimpexNoise), frequencySimplexNoise(frequency),
	amplitdueSimplexNoise(amplitude), octavesSimplexNoise(octaves),
	frequencyChangeSimplex(freqchange), amplitudeChangeSimlpex(ampchange)
{
	PermutationSimplex.resize(256);
	std::iota(PermutationSimplex.begin(), PermutationSimplex.end(), 0);

	// NA PODSTAWIE SEEDA TASUJEMY NASZA TABELE PERMUTACJI 
	std::default_random_engine engine(seedSimplexNoise);
	std::shuffle(PermutationSimplex.begin(), PermutationSimplex.end(), engine);

	// POWIELANIE TABLIY PERMUTACJI -> [0, ..., 255, 0, ..... 255], CHRONI PRZED WYJSCIEM POZA ROZMIAR WEKTORA
	PermutationSimplex.insert(PermutationSimplex.end(), PermutationSimplex.begin(), PermutationSimplex.end());
}

void SimplexNoise::applyToColumn(ChunkColumn& column)
{
	int worldXOffset = column.getX() * 16;
	int worldZOffset = column.getZ() * 16;

	for (int x = 0; x < 16; x++) {
		for (int z = 0; z < 16; z++) {
			float noise = OctavesSimplexFunction(worldXOffset + x, worldZOffset + z, frequencySimplexNoise, amplitdueSimplexNoise
			, octavesSimplexNoise, frequencyChangeSimplex ,amplitudeChangeSimlpex);

			int height = startY + static_cast<int>((noise + 1.0f) * 0.5f * (endY - startY));

			for (int y = 0; y <= height; y++) {
				column.setBlock(x, y, z, 1);
			}
		}
	}
}

void SimplexNoise::renderImGuiSettings()
{
	if (ImGui::TreeNode(layerName.c_str())) {
		ImGui::DragFloat("Czestotliwosc", &frequencySimplexNoise, 0.01f);
		ImGui::DragFloat("Amplituda", &amplitdueSimplexNoise, 0.1f);
		ImGui::DragInt("Oktawy", &octavesSimplexNoise, 1, 1, 8);
		ImGui::DragFloat("Zmiana Freq", &frequencyChangeSimplex, 0.01f);
		ImGui::DragFloat("Zmiana Amp", &amplitudeChangeSimlpex, 0.01f);
		ImGui::DragInt("Start Y", &startY);
		ImGui::DragInt("End Y", &endY);
		ImGui::TreePop();
	}
}

/// Funkcja Hashujaca
int SimplexNoise::Hash(int x, int y)
{
	return PermutationSimplex[PermutationSimplex[x & 255] + (y & 255)];
}

/// FUNKCJA DOTPORUDCT ILOCZYN SKALARNY
float SimplexNoise::DotProduct(float gx, float gy, float x, float y)
{
	return gx * x + gy * y;
}

/// FUNKCJA NOISE'A 
float SimplexNoise::simplexNoise(float x, float y)
{

	//// Simplex Noise 2D JEST "ZBUDOWANY" Z TROJKATOW ROWNOBOCZONYCH -> ALGORYTM LICZY NOISE W KAZDYM Z WIERZCHOLKOW TROJKATA I NA KONIEC GO SUMUJE////
	float Noise1, Noise2, Noise3 = 0.0f; 


	/// STALE UZYWANE W SKEWING I UNSKEWING 
	/// F2 TRANSFORMUJE KWADRATY NA ROMBY (POCHYLA PRZESTRZEN)
	/// G2 TRANSFORMUJE ROMBY W TROJKATY ROWNOBOCZNE
	float F2 = (sqrt(3) - 1) / 2;
	float G2 = (3 - sqrt(3)) / 6;

	/// INFORMACJA W KTORYM KAFELKU (i,j) ZNAJDUJE SIE NASZ PUNKT
	float skew = (x + y) * F2;
	int i = floor(x + skew); 
	int j = floor(y + skew);

	float unskew = (i + j) * G2;

	float X0 = i - unskew; // RZECZYWISTE WSPOLRZEDNE WIERZCHOLKA 0
	float Y0 = j - unskew; 
	float x0 = x - X0; // WEKTOR ODLEGLOSCI PUNKTU (x,y) OD WIERZCHOLKA 0 
	float y0 = y - Y0;

	/// JESTLI x0>y0 TO PUNKT ZNAJDUJE SIE W DOLNYM TROJKACIE SIMPLEXA JESTLI y0 > x0 TO ZNAJDUJE SIE W GORNYM
	int i1, j1;
	if (x0 > y0)
	{
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

	vector2 g0 = Gradients[Hash(i, j) % 12];
	vector2 g1 = Gradients[Hash(i + i1, j + j1) % 12];
	vector2 g2 = Gradients[Hash(i + 1, j + 1) % 12];

	// OBLICZENIA JAK MOCNO WIERZCHOLEK NR 0 WPLYWA NA NOISE CALKOWITY

	float t0 = 0.5f - x0 * x0 - y0 * y0; // t0 to odleg³oœæ
	if (t0 < 0.0f)
		Noise1 = 0.0f;
	else
	{
		//  t0*=t0 -> SZUM BARDZIEJ GLADKI
		t0 *= t0;
		Noise1 = t0 * t0 * DotProduct(g0.x, g0.y, x0, y0);
	}

	// OBLICZENIA JAK MOCNO WIERZCHOLEK NR 1 WPLYWA NA NOISE CALKOWITY

	float t1 = 0.5f - x1 * x1 - y1 * y1;
	if (t1 < 0.0f)
		Noise2 = 0.0f;
	else
	{
		t1 *= t1;
		Noise2 = t1 * t1 * DotProduct(g1.x, g1.y, x1, y1);
	}

	// OBLICZENIA JAK MOCNO WIERZCHOLEK NR 2 WPLYWA NA NOISE CALKOWITY

	float t2 = 0.5f - x2 * x2 - y2 * y2;

	if (t2 < 0.0f)
		Noise3 = 0.0f;
	else
	{
		t2 *= t2;
		Noise3 = t2 * t2 * DotProduct(g2.x, g2.y, x2, y2);
	}

	return 43.0f * (Noise1 + Noise2 + Noise3); // FUNKCJA ZWRACA CALKOWITY NOISE (WSPOLCZYNNIK 43.0f TO STALA NORMALIZACJI MATEMATYCZNEJ DO PRZEDZIALU [-1.0,1.0]


}

/// FUNKCJA WPROWADZA FREQUENCY AMPLITUDE OCTAVES I ZMIANE CZESTOTLIWOSCI I AMPLITUDY 

float SimplexNoise::OctavesSimplexFunction(float x, float y, float frequency, float amplitude, int octaves, float frequencyChange, float amplitudeChange)
{
	float Frequency = frequency;
	float Amplitude = amplitude;
	float octavesNumber = octaves;
	float freqchange = frequencyChange;
	float amplchange = amplitudeChange;
	float totalNoise{};

	for (int oct = 0; octavesNumber > oct; oct++)
	{
		float noiseContribution = simplexNoise(x * Frequency, y * Frequency) * Amplitude;
		/// ZMIENIAM FREQUENCY I AMPLITUDE
		float tempFrequency = Frequency;
		Frequency = tempFrequency * freqchange;
		float tempAmplitude = Amplitude;
		Amplitude = tempAmplitude * amplchange;
		totalNoise = totalNoise + noiseContribution;
	}

	return totalNoise;

}