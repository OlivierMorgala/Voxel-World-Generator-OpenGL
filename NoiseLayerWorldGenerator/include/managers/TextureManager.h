#pragma once
#include <glad/glad.h>
#include <filesystem>

class TextureManager
{
public:
	static unsigned int loadTexture(const std::filesystem::path& filepath);

};

