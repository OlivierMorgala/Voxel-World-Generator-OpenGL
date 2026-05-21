#include "world/WorldRenderer.h"

WorldRenderer::WorldRenderer()
{

}

void WorldRenderer::render(World& world, const Camera& camera, Shader& shader, float windowAspectRatio)
{
	shader.useShader();

	shader.setMatrix4("view", camera.getViewMatrix());
	shader.setMatrix4("projection", camera.getProjectionMatrix(windowAspectRatio));

	int cameraColumnX = static_cast<int>(std::floor(camera.position.x / (Chunk::CHUNK_SIZE)));
	int cameraColumnZ = static_cast<int>(std::floor(camera.position.z / (Chunk::CHUNK_SIZE)));

	for (int x = -config.renderDistance; x <= config.renderDistance; x++) {
		for (int z = -config.renderDistance; z <= config.renderDistance; z++) {
			
			int columnX = cameraColumnX + x;
			int columnZ = cameraColumnZ + z;

			ChunkColumn* column = world.getChunkColumn(columnX, columnZ);

			if (column && column->hasMesh())
			{
				renderChunkColumn(column, shader);
			}

		}
	}
}

void WorldRenderer::renderChunkColumn(const ChunkColumn* column, Shader& shader) const
{
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(column->getX() * Chunk::CHUNK_SIZE, 0, column->getZ() * Chunk::CHUNK_SIZE));
	shader.setMatrix4("model", model);

	column->render(&shader);
}