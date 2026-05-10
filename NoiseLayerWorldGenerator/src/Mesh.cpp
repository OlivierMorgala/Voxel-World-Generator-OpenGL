#include "Mesh.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) :
	VAO(0),
	VBO(0),
	EBO(0)
{
	// Obliczanie liczby indeksów na podstawie rozmiaru wektora indeksów
	indexCount = static_cast<int>(indices.size());
	// Obliczanie liczby wierzchołków na podstawie rozmiaru wektora wierzchołków
	vertexCount = static_cast<int>(vertices.size());

	//Zarezerwowanie pamięci GPU dla VAO, VBO które będą przechowywać dane wierzchołków i konfigurację atrybutów wierzchołków
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	//Aktywacja VAO co pozwala na późniejsze wiązanie tego VAO podczas renderowania aby używać jego konfiguracji atrybutów wierzchołków
	glBindVertexArray(VAO);

	//Przesłanie danych wierzchołków z CPU do GPU poprzez wiązanie VBO i kopiowanie danych wierzchołków do tego bufora, co umożliwia GPU dostęp do tych danych podczas renderowania
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	//Jeśli mamy indeksy, to tworzymy i konfigurujemy EBO (Element Buffer Object) który przechowuje dane indeksów używane do renderowania
	if (indexCount > 0) {
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
	}

	//Przesłanie danych indeksów z CPU do GPU poprzez wiązanie EBO i kopiowanie danych indeksów do tego bufora, co umożliwia GPU dostęp do tych danych podczas renderowania z użyciem indeksów
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	setupAttributes();

	// Odpinanie VAO i VBO po zakończeniu konfiguracji aby zapobiec przypadkowemu modyfikowaniu ich konfiguracji
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::setupAttributes()
{
	//Konfiguracja atrybutów wierzchołków co pozwala GPU zrozumieć jak interpretować dane wierzchołków przechowywane w VBO.
	//Każdy atrybut wierzchołka jest konfigurowany z odpowiednim indeksem, rozmiarem, typem danych, i przesunięciem w strukturze Vertex
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, localPosition));
}

Mesh::~Mesh()
{
	// Zwalnianie zasobów GPU związanych z VAO, VBO i EBO aby uniknąć wycieków pamięci GPU
	if (VAO != 0) { glDeleteVertexArrays(1, &VAO); }
	if (VBO != 0) { glDeleteBuffers(1, &VBO); }
	if (EBO != 0) { glDeleteBuffers(1, &EBO); }
}

void Mesh::draw() const
{
	if (indexCount == 0 && vertexCount == 0) { return; }

	// Renderowanie siatki poprzez wiązanie VAO i wywołanie funkcji rysującej która wykorzystuje liczbę wierzchołków do określenia ile wierzchołków ma zostać narysowanych
	glBindVertexArray(VAO);

	// Jeśli używamy indeksowania wierzchołków (EBO) to rysujemy elementy za pomocą glDrawElements, w przeciwnym razie rysujemy bezpośrednio wierzchołki za pomocą glDrawArrays
	if (indexCount > 0) {
		// Rysowanie z indeksowaniem wierzchołków
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	} else {
		// Rysowanie bez indeksowania wierzchołków
		glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	}

	// Odpinanie VAO po zakończeniu renderowania
	glBindVertexArray(0);
}
