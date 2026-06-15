#pragma once
#include "Raycasting.h"

class BlockPlaceDestroy
{
private:
	Raycast* raycast;
public:
		BlockPlaceDestroy(Raycast* Raycast);
		void PlaceBlock(glm::vec3 BlockPosition);
		void DestroyBlock(glm::vec3 BlockPosition);
};