#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "world/World.h"
#include <algorithm>
#include <iostream>

// Klasa Raycast
class Raycast
{
private:
	// Wskazniki do World i Shader
	World* world;
	Shader* shader; 
public:
	// Zmienne Sluzace do zmniejszenia czestosci wykonywania raycasta
	std::chrono::steady_clock::time_point lastHitTime;
	std::chrono::milliseconds hitCooldown{ 30 };

	// Zmienne do Raycasta
	float maxDistance; 
	bool BlockHit;
	glm::vec3 HitBlockPosition;

	// Metody
	Raycast(float MaxDistance, World* World, Shader* Shader);
	~Raycast() = default;
	void RaycastDDA(const glm::vec3& CameraPosition, const glm::vec3& CameraFront);
	World* GetWorld(); 
};
