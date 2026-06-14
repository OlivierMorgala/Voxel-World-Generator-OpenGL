#include "BlockPlaceDestroy.h"

BlockPlaceDestroy::BlockPlaceDestroy(Raycast* Raycast) : raycast(Raycast) {}

void BlockPlaceDestroy::DestroyBlock(glm::vec3 BlockPosition)
{
	BlockID BlockID = raycast->GetWorld()->getBlock(BlockPosition.x, BlockPosition.y, BlockPosition.z);
	int chunkSize = 16;
	int x = static_cast<int>(std::floor((double)raycast->HitBlockPosition.x / chunkSize)); // WSPOLRZEDNE CHUNKU
	int z = static_cast<int>(std::floor((double)raycast->HitBlockPosition.z / chunkSize));
	if (BlockID != 0)
	{
		/// JESLI BLOK TO KAMIEN LUB ZIEMIA TO MOZEMY GO ZNISZCZYC
		raycast->GetWorld()->setBlock(BlockPosition.x, BlockPosition.y, BlockPosition.z, 0);
		auto column = raycast->GetWorld()->getChunkColumn(x, z);
		raycast->GetWorld()->renderAlteredChunks(column);
	}

	int localX = BlockPosition.x - (x * chunkSize);
	int localZ = BlockPosition.z - (z * chunkSize);

	if (localX == 0)
	{
		auto neighbour = raycast->GetWorld()->getChunkColumn(x - 1, z);
		raycast->GetWorld()->renderAlteredChunks(neighbour);
	}
	else if (localX == 15)
	{
		auto neighbour = raycast->GetWorld()->getChunkColumn(x + 1, z);
		raycast->GetWorld()->renderAlteredChunks(neighbour);
	}
	//  TO SAMO ROBIMY Z OSIA Z
	if (localZ == 0)
	{
		auto neighbour = raycast->GetWorld()->getChunkColumn(x, z - 1);
		raycast->GetWorld()->renderAlteredChunks(neighbour);
	}
	else if (localZ == 15)
	{
		auto neighbour = raycast->GetWorld()->getChunkColumn(x, z + 1);
		raycast->GetWorld()->renderAlteredChunks(neighbour);
	}
}

