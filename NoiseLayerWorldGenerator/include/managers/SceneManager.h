#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include"scenes\Scene.h"
#include <memory>
#include <vector>

class SceneManager
{
private:
	std::vector<std::unique_ptr<Scene>> scenes;
	std::unique_ptr<Scene> pendingPush;
	bool pendingPop = false;


	SceneManager() = default;
	~SceneManager() = default;
public:
	static SceneManager& getInstance();

	void pushScene(std::unique_ptr<Scene> newScene);

	void popScene();

	//Metoda służoca do aktualizacji logiki aktualnej sceny przyjmująca czas deltaTime jako parametr który reprezentuje czas jaki upłynął od ostatniej aktualizacji. Ta metoda będzie wywoływana w każdej klatce aby zapewnić płynną aktualizację logiki gry
	void update(float deltaTime);

	//Metoda służoca do renderowania aktualnej sceny. Ta metoda będzie wywoływana w każdej klatce aby narysować aktualny stan gry na ekranie
	void render();
};

