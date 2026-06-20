#include "scenes\Scene.h"

// Podstawowa klasa wszystkich ekranów - chwyta tylko okno
Scene::Scene() {
	window = WindowManager::getInstance().getMainWindow();
}

