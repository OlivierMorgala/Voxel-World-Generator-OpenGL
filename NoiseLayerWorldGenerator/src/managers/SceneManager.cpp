#include "managers\SceneManager.h"

SceneManager& SceneManager::getInstance() {
	static SceneManager instance;
	return instance;
}

void SceneManager::pushScene(std::unique_ptr<Scene> newScene) {
	pendingPush = std::move(newScene);
}

void SceneManager::popScene() {
	pendingPop = true;
}

void SceneManager::update(float deltaTime) {
	if (pendingPop && !scenes.empty()) {
		scenes.back()->onExit();
		scenes.pop_back();
		pendingPop = false;
	}

	while (pendingPush) {
		std::unique_ptr<Scene> newScene = std::move(pendingPush);
		scenes.push_back(std::move(newScene));
		scenes.back()->onEnter();
	}

	if (!scenes.empty()) {
		scenes.back()->onUpdate(deltaTime);
	}
}

void SceneManager::render() {
	// Jeśli istnieje aktualna scena wywołujemy jej metodę Render() w celu narysowania aktualnego stanu sceny na ekranie
	if (!scenes.empty()) {
		scenes.back()->render();
	}

	//Incijalizujemy nową klatkę dla ImGui, co pozwala na rysowanie interfejsu użytkownika w każdej klatce po renderowaniu sceny.
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//Dodanie elementów intyerfejsu dla danej sceny, jeśli metoda onImGuiRender() została nadpisana w klasie sceny.
	if (!scenes.empty()) {
		scenes.back()->onImGuiRender();
	}

	// Kończymy klatkę ImGui i renderujemy interfejs użytkownika na ekranie
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}