#include "scenes/LoadingScene.h"
#include "scenes/WorldGeneratorScene.h"
#include "managers/SceneManager.h"
#include "managers/TextureManager.h"

LoadingScene::LoadingScene(World* loadedWorld) : world(loadedWorld)
{

}

void LoadingScene::onEnter()
{
	std::cout << "+[Scene] LOADED: LoadingScene" << std::endl;
	planetTextureID = TextureManager::loadTexture("assets/textures/planet-sheet.png");
}

void LoadingScene::onExit()
{
	std::cout << "-[Scene] UNLOADED: LoadingScene" << std::endl;

	if (planetTextureID != 0) {
		glDeleteTextures(1, &planetTextureID);
	}
}

void LoadingScene::onUpdate(float deltaTime)
{
	if (world) {
		world->updateWorld();

		// Jeśli świat się zbudował i wszedł w tryb grania, zdejmujemy loading
		if (world->getCurrentState() == WorldState::PLAYING) {
			SceneManager::getInstance().popScene();
		}
	}

	// Aktualizacja klatek animacji obracającej się planety
	animationTimer += deltaTime;
	if (animationTimer >= frameDuration) {
		animationTimer -= frameDuration;
		currentFrame = (currentFrame + 1) % totalFrames;
	}
}

void LoadingScene::render()
{
	// Ciemne tło na ekranie ładowania
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// UI ładowania (Napis i pasek postępu)
void LoadingScene::onImGuiRender()
{
	// Środek ekranu
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoInputs;

	if (ImGui::Begin("Loading", nullptr, windowFlags)) {

		float contentWidth = 400.0f;
		ImGui::SetNextWindowSize(ImVec2(contentWidth, 0), ImGuiCond_Always);

		// Obliczanie klatek w locie ze sprite sheet'a dla animacji
		if (planetTextureID != 0) {
			float framePixelWidth = 1330.0f / static_cast<float>(cols);
			float framePixelHeight = 6288.0f / static_cast<float>(rows);

			float imageWidth = 265.0f;
			float imageHeight = imageWidth * (framePixelHeight / framePixelWidth);

			int currentCol = currentFrame % cols;
			int currentRow = currentFrame / cols;

			float u_step = 1.0f / static_cast<float>(cols);
			float v_step = 1.0f / static_cast<float>(rows);

			float u0 = currentCol * u_step;
			float u1 = u0 + u_step;
			float v0 = 1.0f - (currentRow * v_step); // Oś V trzeba odwrócić
			float v1 = v0 - v_step;

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f);
			ImGui::SetCursorPosX((contentWidth - imageWidth) * 0.5f);
			ImGui::Image((void*)(intptr_t)planetTextureID, ImVec2(imageWidth, imageHeight), ImVec2(u0, v0), ImVec2(u1, v1));

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 60.0f);
			ImGui::Spacing();
		}

		ImGui::Text("Generating world...");
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
		ImGui::Separator();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);

		// Odpytywanie silnika o postęp generacji
		float progress = 0.0f;
		if (world) {
			progress = world->getGenerationProgress();
		}

		// Rendering klasycznego paska progress bar
		ImGui::ProgressBar(progress, ImVec2(400.0f, 30.0f));
	}
	ImGui::End();
}
