#include "managers\SceneManager.h"

// Wzorzec Singleton - tylko jedna instancja
SceneManager& SceneManager::getInstance() {
	static SceneManager instance;
	return instance;
}

// Planowanie wejścia nowej sceny (bezpieczne odroczenie na później)
void SceneManager::pushScene(std::unique_ptr<Scene> newScene) {
	pendingPush = std::move(newScene);
}

// Planowanie opuszczenia obecnej sceny
void SceneManager::popScene() {
	pendingPop = true;
}

// Główna funkcja aktualizująca, odpala zmiany zaplanowane wyżej
void SceneManager::update(float deltaTime) {

	// Ściąganie sceny ze stosu
	if (pendingPop && !scenes.empty()) {
		scenes.back()->onExit();
		scenes.pop_back();
		pendingPop = false;
	}

	// Wrzucanie nowej sceny na wierzch
	while (pendingPush) {
		std::unique_ptr<Scene> newScene = std::move(pendingPush);
		scenes.push_back(std::move(newScene));
		scenes.back()->onEnter();
	}

	// Puszczanie update'u dla tej na samej górze
	if (!scenes.empty()) {
		scenes.back()->onUpdate(deltaTime);
	}
}

// Ogólny render gry
void SceneManager::render() {

	// Najpierw grafika świata/sceny (OpenGL)
	if (!scenes.empty()) {
		scenes.back()->render();
	}

	// Przygotowanie klatki interfejsu
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Render ImGui z przypisanej sceny
	if (!scenes.empty()) {
		scenes.back()->onImGuiRender();
	}

	// Wyrysowanie UI na ekran
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
