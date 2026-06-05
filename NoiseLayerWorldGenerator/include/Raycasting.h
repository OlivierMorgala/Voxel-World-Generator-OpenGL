#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "world/World.h"
#include <algorithm>
#include <iostream>


class Raycast
{
private:
	World* world;
public:
	float maxDistance;
	bool BlockHit;
	glm::vec3 HitBlockPosition;
	Raycast(float MaxDistance, World* World);
	~Raycast() = default;
	void RaycastDDA(const glm::vec3& CameraPosition, const glm::vec3& CameraFront);
};
