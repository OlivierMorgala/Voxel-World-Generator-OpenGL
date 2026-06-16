#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

class Shader
{
private:
	//ID programu shaderów
	unsigned int ID;

	//Metoda pomocnicza do sprawdzania błędów kompilacji shaderów
	void checkCompileErrors(unsigned int shader, const std::string& type);

public:
	//Konstruktor który tworzy program shaderów na podstawie ścieżek do plików z kodem shaderów
	Shader(const char* vertexPath, const char* fragmentPath);

	//Metoda aktywuje dany program shaderów dla GPU co pozwala na używanie tego programu shaderów podczas renderowania obiektów w scenie
	void useShader();

	//Metdoda pomocnicza które pozwalaja na przekazywanie danych z CPU do GPU poprzez uniformy shaderów umożliwiając dynamiczną kontrolę nad zachowaniem shaderów podczas renderowania
	void setValue(const std::string& name, float value) const;

	void setValueVec3(const std::string& name, glm::vec3& value) const;

	void setMatrix4(const std::string& name, const glm::mat4& mat) const;

	unsigned int getID();
};

