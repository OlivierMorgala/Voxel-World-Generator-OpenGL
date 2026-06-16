#include "world/WorldRenderer.h"

WorldRenderer::WorldRenderer()
{
	visibleColumns.reserve(3000);
}

void WorldRenderer::render(World& world, const Camera& camera, Shader& shader, float windowAspectRatio)
{

	shader.useShader();

	shader.setMatrix4("view", camera.getViewMatrix());
	shader.setMatrix4("projection", camera.getProjectionMatrix(windowAspectRatio));

	int cameraColumnX = static_cast<int>(std::floor(camera.position.x / (Chunk::CHUNK_SIZE)));
	int cameraColumnZ = static_cast<int>(std::floor(camera.position.z / (Chunk::CHUNK_SIZE)));

	isCameraUnderwater = false;
	underwaterColor = glm::vec3(0.0f);

	int cameraBlockX = static_cast<int>(std::floor(camera.position.x));
	int cameraBlockY = static_cast<int>(std::floor(camera.position.y));
	int cameraBlockZ = static_cast<int>(std::floor(camera.position.z));

	std::shared_ptr<ChunkColumn> cameraColumn = world.getChunkColumn(cameraColumnX, cameraColumnZ);
	if (cameraColumn) {
		int localX = cameraBlockX % Chunk::CHUNK_SIZE;
		if(localX < 0) { localX += Chunk::CHUNK_SIZE; }
		int localZ = cameraBlockZ % Chunk::CHUNK_SIZE;
		if (localZ < 0) { localZ += Chunk::CHUNK_SIZE; }

		BlockID currentBlockID = cameraColumn->getBlock(localX, cameraBlockY, localZ);

		if(currentBlockID != 0 && BlockDatabase::getBlockData(currentBlockID).isTransparent){
			isCameraUnderwater = true;
			underwaterColor = BlockDatabase::getBlockData(currentBlockID).color;
		}
	}

	shader.setValue("alpha", 1.0f);
	shader.setValue("isBorderRendered", true);
	shader.setValue("showColumnBorders", config.showChunkColumnsBorder);
	shader.setValue("chunkSize", static_cast<float>(Chunk::CHUNK_SIZE));
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	visibleColumns.clear();

	Frustum frustumCamera;
	if (config.isFrustumCullingEnabled) {
		frustumCamera = camera.getFrustum(windowAspectRatio);
	}

	std::vector<ChunkColumn*> loadedColumns = world.getLoadedColumns();

	for (ChunkColumn* column : loadedColumns) {

		int dx = column->getX() - cameraColumnX;
		int dz = column->getZ() - cameraColumnZ;

		if (std::abs(dx) <= config.renderDistance && std::abs(dz) <= config.renderDistance) {
			if (column->hasMesh()) {
				bool isVisible = true;

				if (config.isFrustumCullingEnabled) {
					AA_BoundingBox AABBcolumn;

					AABBcolumn.min.x = column->getX() * Chunk::CHUNK_SIZE;
					AABBcolumn.min.y = 0.0f;
					AABBcolumn.min.z = column->getZ() * Chunk::CHUNK_SIZE;

					AABBcolumn.max.x = AABBcolumn.min.x + Chunk::CHUNK_SIZE;
					AABBcolumn.max.y = config.worldHeightInChunks * Chunk::CHUNK_SIZE;
					AABBcolumn.max.z = AABBcolumn.min.z + Chunk::CHUNK_SIZE;

					isVisible = camera.isAABoundingBoxVisible(frustumCamera, AABBcolumn);

				}

				if (isVisible) {
					column->renderOpaque(&shader);
					visibleColumns.push_back(column);
				}
			}
		}
	}

	shader.setValue("alpha", 0.65f);
	shader.setValue("isBorderRendered", false);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);

	for (ChunkColumn* column : visibleColumns) {
		column->renderTransparent(&shader);
	}

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
}

const std::vector<ChunkColumn*>& WorldRenderer::getVisibleColumns() const 
{
	return visibleColumns;
}