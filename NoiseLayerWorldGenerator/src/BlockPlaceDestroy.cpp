#include "BlockPlaceDestroy.h"

BlockPlaceDestroy::BlockPlaceDestroy(Raycast* Raycast) : raycast(Raycast) {}

void BlockPlaceDestroy::DestroyBlock(glm::vec3 BlockPosition)
{
	BlockID BlockID = raycast->GetWorld()->getBlock(BlockPosition.x, BlockPosition.y, BlockPosition.z);

	if (BlockID == 1 || BlockID == 2)
	{
		/// JESLI BLOK TO KAMIEN LUB ZIEMIA TO MOZEMY GO ZNISZCZYC
		raycast->GetWorld()->setBlock(BlockPosition.x, BlockPosition.y, BlockPosition.z, 0);

	}
}