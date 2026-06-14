#pragma once

struct WorldConfig
{
	//GŁÓWNE----------------------

	int worldSeed = 1;
	int renderDistance = 22;

	//KAMERA----------------------

	float cameraSpeed = 50.0f;
	float fov = 45.0f;
	float viewBegin = 0.1f;
	float viewDistance = 1000.0f;

	//ZAAWANSOPWANE----------------

	bool showChunkColumnsBorder = false;
	bool isFrustumCullingEnabled = true;
	bool isHiddenWallCullingEnabled = true;

	//-----------------------------



	int worldHeightInChunks = 16;  //Obecnie program sam ją zmienia na najbardziej optymalną wartość
};

extern WorldConfig config;

