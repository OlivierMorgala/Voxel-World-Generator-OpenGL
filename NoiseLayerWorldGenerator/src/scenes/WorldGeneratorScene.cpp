#include "scenes\WorldGeneratorScene.h"


void WorldGeneratorScene::onEnter()
{
	std::cout << "+[Scene] LOADED: WorldGeneratorScene" << std::endl;

    if (window) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

	// Inicjalizacja kamery na pozycji (0, 0, 1) - tymczasowa wartość później bedzie trzeba ja ustalać wzgledem wytworzonego terenu
	camera = std::make_unique<Camera>(glm::vec3(8.0f, 40.0f, 8.0f));

	worldGenUI = std::make_unique<WorldGeneratorUI>();

	//TYMCZASOWO - test renderowania śiwata
    BlockDatabase::init();

    mainShader = std::make_unique<Shader>("shaders/test.vert", "shaders/test.frag");
    world = std::make_unique<World>();
    worldRenderer = std::make_unique<WorldRenderer>();
    //---
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

    glm::vec2 delta = Input::getMouseDelta();
    camera->processMouseMovement(delta.x, delta.y);

    if (world) {
        world->updateWorld(camera->position);
    }
}

void WorldGeneratorScene::render()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Ustawienie macierzy widoku kamery w shaderze
	glm::mat4 view = camera->getViewMatrix();
	mainShader->setMatrix4("view", view);

    //TYMCZASOWO - test ładowania shaderów
    if (!world || !worldRenderer || !mainShader) return;

	GLFWwindow* window = WindowManager::getInstance().getMainWindow();

    if (window) {
        int width;
        int height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        worldRenderer->render(*world, *camera, *mainShader, aspectRatio);
    }
    //---
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