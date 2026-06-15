#include "Shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	//Zmienne do przechowywania kodu shaderów
	std::string vertexCode;
	std::string fragmentCode;

	//Zmienne do obsługi plików shaderów
	std::ifstream vertexShaderFile;
	std::ifstream fragmentShaderFile;

	//Ustawienie wyjątków dla obsługi błędów podczas odczytu plików shaderów
	vertexShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fragmentShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		//Otwieranie plików shaderów i odczytywanie ich zawartości do stringstreamów
		vertexShaderFile.open(vertexPath);
		fragmentShaderFile.open(fragmentPath);
		std::stringstream vertexShaderStream;
		std::stringstream fragmentShaderStream;
		vertexShaderStream << vertexShaderFile.rdbuf();
		fragmentShaderStream << fragmentShaderFile.rdbuf();
		vertexShaderFile.close();
		fragmentShaderFile.close();

		//Konwersja zawartości stringstreamów do stringów
		vertexCode = vertexShaderStream.str();
		fragmentCode = fragmentShaderStream.str();
	}
	catch (std::ifstream::failure& e) {
		//Obsługa błędów podczas odczytu plików shaderów
		std::cout << "[ERROR::SHADER] -> ERROR READING SHADER FILES" << std::endl;
	}

	//Konwersja stringów z kodem shaderów do formatów akceptowanych przez OpenGL
	const char* vertexShaderCode = vertexCode.c_str();
	const char* fragmentShaderCode = fragmentCode.c_str();

	//zmienne do przechowywania identyfikatorów shaderów
	unsigned int vertexShader;
	unsigned int fragmentShader;

	//Tworzenie, kompilacja, i sprawdzanie błędów dla shaderów vertex
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
	glCompileShader(vertexShader);
	checkCompileErrors(vertexShader, "VERTEX");

	//Tworzenie, kompilacja, i sprawdzanie błędów dla shaderów fragment
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
	glCompileShader(fragmentShader);
	checkCompileErrors(fragmentShader, "FRAGMENT");

	//Tworzenie programu shaderów, dołączanie shaderów, linkowanie programu, i sprawdzanie błędów
	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");

	//Usuwanie shaderów po ich dołączeniu do programu ponieważ nie są już potrzebne po linkowaniu programu shaderów i mogą być usunięte aby zwolnić zasoby GPU
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

unsigned int Shader::getID()
{
	return ID;
}

void Shader::useShader()
{
	glUseProgram(ID);
}

void Shader::setValue(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setValueVec3(const std::string& name, glm::vec3& value) const
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()),value.x, value.y, value.z);
}

void Shader::setMatrix4(const std::string& name, const glm::mat4& matrix) const {
	//Pobieramy lokalizację uniformu o podanej nazwie w programie shaderów. Ta lokalizacja jest potrzebna do późniejszego ustawienia wartości tego uniformu
	int location = glGetUniformLocation(ID, name.c_str());

	//Wysyłamy 1 macierz 4x4 do GPU jako wartość uniformu o podanej lokalizacji. GL_FALSE oznacza że macierz nie jest transponowana podczas przesyłania do GPU a glm::value_ptr(mat) zwraca wskaźnik do danych macierzy w formacie akceptowanym przez OpenGL
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::checkCompileErrors(unsigned int shader, const std::string& type)
{
	//Zmienne do przechowywania statusu kompilacji/linkowania i logów błędów
	int success;
	char infoLog[1024];

	//Sprawdzanie błędów kompilacji dla shaderów vertex i fragment oraz błędów linkowania dla programu shaderów i wypisywanie odpowiednich komunikatów o błędach w przypadku niepowodzenia
	if(type != "PROGRAM") {
		//Sprawdzanie statusu kompilacji shaderów vertex i fragment
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

		//Jeśli kompilacja się nie powiodła pobieramy logi błędów i wypisujemy komunikatu o błędzie
		if(!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "[ERROR::SHADER] -> COMPILATION ERROR OF TYPE: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else {
		//Sprawdzanie statusu linkowania programu shaderów
		glGetProgramiv(shader, GL_LINK_STATUS, &success);

		//Jeśli linkowanie się nie powiodło pobieramy logi błędów i wypisujemy komunikatu o błędzie
		if(!success) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "[ERROR::SHADER] -> LINKING ERROR OF TYPE: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	
}
