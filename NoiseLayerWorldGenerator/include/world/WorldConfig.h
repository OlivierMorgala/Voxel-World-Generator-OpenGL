#pragma once

struct WorldConfig
{
	float cameraSpeed = 3.0f;
	float fov = 45.0f;
	float viewBegin = 0.1f;
	float viewDistance = 1000.0f;

	int worldSeed = 0;
	int worldHeightInChunks = 16;
	int renderDistance = 22;
};

extern WorldConfig config;

