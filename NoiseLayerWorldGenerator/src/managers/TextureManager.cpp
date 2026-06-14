#include "managers/TextureManager.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

unsigned int TextureManager::loadTexture(const std::filesystem::path& filepath) 
{

	if (!std::filesystem::exists(filepath)) {
		std::cout << "[TextureManager] Plik tekstury nie istnieje: " << filepath.string() << "\n";
		return 0;
	}

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width;
	int height;
	int componentsNumber;

	stbi_set_flip_vertically_on_load(true);

	std::string pathString = filepath.string();
	unsigned char* data = stbi_load(pathString.c_str(), &width, &height, &componentsNumber, 0);

	if (data) {
		GLenum format = GL_RGB;
		if (componentsNumber == 1) { format = GL_RED; }
		else if (componentsNumber == 3) { format = GL_RGB; }
		else if(componentsNumber == 4) { format = GL_RGBA; }

		glBindTexture(GL_TEXTURE_2D, textureID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		stbi_image_free(data);
	}
	else 
	{
		std::cout << "[TextureManager] Nie udalo sie wczytac tekstury z pliku:  " << filepath.string() << "\n";
		stbi_image_free(data);
	}

	return textureID;
}

