#include "scenes\WorldGeneratorScene.h"
#include "scenes\LoadingScene.h"
#include "managers\SceneManager.h"
#include "world\generationAlgorithms\PerlinNoise2D.h"
#include "world\generationAlgorithms\FlatFill.h"

void WorldGeneratorScene::onEnter()
{
	std::cout << "+[Scene] LOADED: WorldGeneratorScene" << std::endl;

    if (window) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

	// Inicjalizacja kamery na pozycji (0, 0, 1) - tymczasowa wartość później bedzie trzeba ja ustalać wzgledem wytworzonego terenu
	camera = std::make_unique<Camera>(glm::vec3(8.0f, 40.0f, 8.0f));

	//TYMCZASOWO
    BlockDatabase::init();
    //---

    mainShader = std::make_unique<Shader>("shaders/test.vert", "shaders/test.frag");
    
	worldTerrainGenerator = std::make_unique<WorldTerrainGenerator>();

	worldTerrainGenerator->generationLayers.push_back(std::make_unique<FlatFill>("Flat", 60, 70));
    worldTerrainGenerator->generationLayers.push_back(std::make_unique<PerlinNoise2D>("Perlin", 5, 50, 12345, 0.090, 0.700, 6, 1.740, 0.400));

    world = std::make_unique<World>();
    world->setCamera(camera.get());
    world->setTerrainGenerator(worldTerrainGenerator.get());

	worldRenderer = std::make_unique<WorldRenderer>();
    worldGenUI = std::make_unique<WorldGeneratorUI>(worldTerrainGenerator.get(), world.get());

	world->regenerateWorld();
	SceneManager::getInstance().pushScene(std::make_unique<LoadingScene>(world.get()));
}



void WorldGeneratorScene::onExit()
{
    if (window) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
	std::cout << "-[Scene] UNLOADED: WorldGeneratorScene" << std::endl;
}



void WorldGeneratorScene::onUpdate(float deltaTime)
{
	if (!camera || !window) { return; }

    if (Input::isKeyPressed(GLFW_KEY_W)) camera->processKeyboardInput(Camera::FORWARD, deltaTime);
    if (Input::isKeyPressed(GLFW_KEY_S)) camera->processKeyboardInput(Camera::BACKWARD, deltaTime);
    if (Input::isKeyPressed(GLFW_KEY_A)) camera->processKeyboardInput(Camera::LEFT, deltaTime);
    if (Input::isKeyPressed(GLFW_KEY_D)) camera->processKeyboardInput(Camera::RIGHT, deltaTime);
    if (Input::isKeyPressed(GLFW_KEY_LEFT_SHIFT)) camera->processKeyboardInput(Camera::DOWN, deltaTime);
    if (Input::isKeyPressed(GLFW_KEY_SPACE)) camera->processKeyboardInput(Camera::UP, deltaTime);

    if (Input::isKeyJustPressed(GLFW_KEY_ESCAPE)) {
        isCursorMode = !isCursorMode;
        glfwSetInputMode(window, GLFW_CURSOR, isCursorMode ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

    if (!isCursorMode) {
        glm::vec2 delta = Input::getMouseDelta();
        camera->processMouseMovement(delta.x, delta.y);
    }

    if (world) {
        world->updateWorld();
    }
}



void WorldGeneratorScene::render()
{
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (!world || !worldRenderer || !mainShader) return;

    glm::mat4 view = camera->getViewMatrix();
	mainShader->setMatrix4("view", view);


    if (window) {
        int width;
        int height;
        glfwGetFramebufferSize(window, &width, &height);

        if (height == 0) { height = 1; }
        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);


        if (world->getCurrentState() == WorldState::PLAYING) {
            worldRenderer->render(*world, *camera, *mainShader, aspectRatio);
        }
    }
}



void WorldGeneratorScene::onImGuiRender()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.3f);

    if (ImGui::Begin("CORDHUD", nullptr, window_flags)) {
        if (camera) {
            ImGui::Text("X: %.2f", camera.get()->position.x);
            ImGui::SameLine();
            ImGui::Text(" | ");
            ImGui::SameLine();
            ImGui::Text("Y: %.2f", camera.get()->position.y);
            ImGui::SameLine();
            ImGui::Text(" | ");
            ImGui::SameLine();
            ImGui::Text("Z: %.2f", camera.get()->position.z);
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        }
        ImGui::Separator();
    }
    ImGui::End();

    if (worldGenUI) {
        worldGenUI->renderImGui();
    }
}