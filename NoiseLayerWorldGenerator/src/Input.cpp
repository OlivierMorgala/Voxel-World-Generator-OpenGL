#include "Input.h"
#include "managers/InputManager.h"
#include <imgui.h> 

// Skrócone odwołania do głównego Input Managera
bool Input::isKeyPressed(int key)
{
	return InputManager::isKeyPressed(key);
}

bool Input::isKeyJustPressed(int key)
{
	return InputManager::isKeyJustPressed(key);
}

bool Input::isMouseButtonPressed(int button)
{
	return InputManager::isMouseButtonPressed(button);
}

glm::vec2 Input::getMousePosition()
{
	return InputManager::getMousePosition();
}

glm::vec2 Input::getMouseDelta()
{
	return InputManager::getMouseDelta();
}

// Sprawdzenie, czy kursor aktualnie nakierowany jest na element UI
// Chroni to przed przypadkowym kopaniem dołu/strzelaniem, gdy się klika np. na suwak
bool Input::isMouseOverUI()
{
	return ImGui::GetIO().WantCaptureMouse;
}

// Sprawdzenie, czy gracz wpisuje właśnie jakiś tekst w okienku ImGui
bool Input::isKeyboardInputCapturedByUI()
{
	return ImGui::GetIO().WantCaptureKeyboard;
}