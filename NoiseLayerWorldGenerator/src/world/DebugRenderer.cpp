#include "world/DebugRenderer.h"

DebugRenderer::DebugRenderer() 
{
	borderShader = std::make_unique<Shader>("shaders/border.vert", "shaders/border.frag");
	initBorderMesh();
}



DebugRenderer::~DebugRenderer()
{
	if (borderVBO != 0) { glDeleteBuffers(1, &borderVBO); }
	if (borderVAO != 0) { glDeleteVertexArrays(1, &borderVAO); }
}



void DebugRenderer::initBorderMesh() 
{
	std::vector<float> vertices;
	float  width = static_cast<float>(Chunk::CHUNK_SIZE);
	float height = static_cast<float>(config.worldHeightInChunks * Chunk::CHUNK_SIZE);

	vertices.insert(vertices.end(), { 0,0,0,0,height,0 });
	vertices.insert(vertices.end(), { width,0,0,width,height,0 });
	vertices.insert(vertices.end(), { width,0,width,width,height,width });
	vertices.insert(vertices.end(), { 0,0,width,0,height,width });


	for (float y = 0; y <= height; y += width) {
		vertices.insert(vertices.end(), { 0,y,0,width,y,0 });
		vertices.insert(vertices.end(), { width,y,0,width,y,width });
		vertices.insert(vertices.end(), { width,y,width,0,y,width });
		vertices.insert(vertices.end(), { 0,y,0,0,y,width });
	}

	borderVertexCount = vertices.size() / 3;

	glGenVertexArrays(1, &borderVAO);
	glGenBuffers(1, &borderVBO);

	glBindVertexArray(borderVAO);
	glBindBuffer(GL_ARRAY_BUFFER, borderVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}



void DebugRenderer::renderChunkBorders(const std::vector<ChunkColumn*>& visibleColumns, const Camera& camera, float aspectRatio) 
{
	if (visibleColumns.empty()) { return; }

	borderShader->useShader();
	borderShader->setMatrix4("view", camera.getViewMatrix());
	borderShader->setMatrix4("projection", camera.getProjectionMatrix(aspectRatio));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glLineWidth(2.0f);

	glBindVertexArray(borderVAO);

	for (const ChunkColumn* column : visibleColumns) {
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(column->getX() * Chunk::CHUNK_SIZE, 0, column->getZ() * Chunk::CHUNK_SIZE));
		borderShader->setMatrix4("model", model);

		glDrawArrays(GL_LINES, 0, borderVertexCount);
	}

	glBindVertexArray(0);
	glLineWidth(1.0f);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}