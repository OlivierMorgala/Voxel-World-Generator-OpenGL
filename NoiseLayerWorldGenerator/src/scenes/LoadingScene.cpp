#include "scenes/LoadingScene.h"
#include "scenes/WorldGeneratorScene.h"
#include "managers/SceneManager.h"

LoadingScene::LoadingScene(World* loadedWorld) : world(loadedWorld)
{

}

void LoadingScene::onEnter()
{
	std::cout << "+[Scene] LOADED: LoadingScene" << std::endl;
}

void LoadingScene::onExit()
{
	std::cout << "-[Scene] UNLOADED: LoadingScene" << std::endl;
}

void LoadingScene::onUpdate(float deltaTime)
{
	if (world) {
		world->updateWorld();

		if (world->getCurrentState() == WorldState::PLAYING) {
			SceneManager::getInstance().popScene();
		}
	}
}

void LoadingScene::render()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void LoadingScene::onImGuiRender()
{
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoInputs;

	if (ImGui::Begin("Loading", nullptr, windowFlags)) {
			ImGui::Text("Generating world...");

			ImGui::Separator();

			float progress = 0.0f;
			if (world) {
				progress = world->getGenerationProgress();
			}

			ImGui::ProgressBar(progress, ImVec2(400.0f, 30.0f));
	}
	ImGui::End();
}