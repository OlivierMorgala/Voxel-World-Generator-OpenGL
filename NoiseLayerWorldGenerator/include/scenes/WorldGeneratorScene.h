#pragma once
#include "scenes/Scene.h"
#include <iostream>
#include "imgui.h"
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"
#include "Input.h"
#include "world/World.h"
#include "world/WorldRenderer.h"
#include "world/DebugRenderer.h"
#include "world/WorldTerrainGenerator.h"
#include "managers/WindowManager.h"
#include "gui/WorldGeneratorUI.h"

class WorldGeneratorScene : public Scene
{
public:
	WorldGeneratorScene(std::vector<TerrainLayer> initialLayers = {});
	~WorldGeneratorScene() override = default;

	void onEnter() override;
	void onExit() override;
	void onUpdate(float deltaTime) override;
	void render() override;

	void onImGuiRender() override;

private:
	std::vector<TerrainLayer> presetLayers;

	std::unique_ptr<Camera> camera;
	//Flaga która sprawdza czy sterujemy myszą lub kamerą
	bool isCursorMode = false;

	std::unique_ptr<Shader> mainShader;

	std::unique_ptr<World> world;
	std::unique_ptr<WorldTerrainGenerator> worldTerrainGenerator;
	std::unique_ptr<WorldRenderer> worldRenderer;
	std::unique_ptr<DebugRenderer> debugRenderer;

	std::unique_ptr<WorldGeneratorUI> worldGenUI;
};

