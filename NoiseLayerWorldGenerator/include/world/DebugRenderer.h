#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include "Shader.h"
#include "Camera.h"
#include "world/ChunkColumn.h"
#include "world/Chunk.h"

class DebugRenderer
{
private:
	std::unique_ptr<Shader> borderShader;
	unsigned int borderVAO = 0;
	unsigned int borderVBO = 0;
	int borderVertexCount = 0;

	void initBorderMesh();

public:
	DebugRenderer();
	~DebugRenderer();

	void renderChunkBorders(const std::vector<ChunkColumn*>& visibleColumns, const Camera& camera, float aspectRatio);
};

