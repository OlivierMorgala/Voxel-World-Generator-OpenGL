#include "BlockPlaceDestroy.h"

BlockPlaceDestroy::BlockPlaceDestroy(Raycast* Raycast) : raycast(Raycast) {}

void BlockPlaceDestroy::DestroyBlock(glm::vec3 BlockPosition)
{
	BlockID BlockID = raycast->GetWorld()->getBlock(BlockPosition.x, BlockPosition.y, BlockPosition.z);

	if (BlockID == 1 || BlockID == 2)
	{
		/// JESLI BLOK TO KAMIEN LUB ZIEMIA TO MOZEMY GO ZNISZCZYC
		raycast->GetWorld()->setBlock(BlockPosition.x, BlockPosition.y, BlockPosition.z, 0);
		int chunkSize = 16;
		int x = static_cast<int>(std::floor((double)raycast->HitBlockPosition.x / chunkSize)); // WSPOLRZEDNE CHUNKU
		int z = static_cast<int>(std::floor((double)raycast->HitBlockPosition.z / chunkSize));

		auto column = raycast->GetWorld()->getChunkColumn(x, z);
		raycast->GetWorld()->renderAlteredChunks(column);
	}
}

