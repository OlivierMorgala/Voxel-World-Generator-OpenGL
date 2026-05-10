#include "Input.h"
#include "managers/InputManager.h"
#include <imgui.h> 


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

bool Input::isMouseOverUI()
{
	return ImGui::GetIO().WantCaptureMouse;
}