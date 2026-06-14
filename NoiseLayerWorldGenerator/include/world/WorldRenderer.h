#pragma once

#include "world/World.h"
#include "Camera.h"
#include "Shader.h"

class WorldRenderer
{
private:
	std::vector<ChunkColumn*> visibleColumns;

public:
	WorldRenderer();
	~WorldRenderer() = default;

	bool isCameraUnderwater = false;
	glm::vec3 underwaterColor = glm::vec3(0.0f);

	void render(World& world, const Camera& camera, Shader& shader, float windowAspectRatio);

	const std::vector<ChunkColumn*>& getVisibleColumns() const;
};

