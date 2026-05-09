#include "managers/InputManager.h"

bool InputManager::keyStates[GLFW_KEY_LAST] = { false };
bool InputManager::prevKeyStates[GLFW_KEY_LAST] = { false };
bool InputManager::mouseButtonStates[GLFW_MOUSE_BUTTON_LAST] = { false };
bool InputManager::prevMouseButtonStates[GLFW_MOUSE_BUTTON_LAST] = { false };

double InputManager::mouseX = 0.0;
double InputManager::mouseY = 0.0;
double InputManager::lastMouseX = 0.0;
double InputManager::lastMouseY = 0.0;

glm::vec2 InputManager::sensitivity = glm::vec2(0.05f, 0.05f);

void InputManager::updateKeyStates() {
	std::memcpy(prevKeyStates, keyStates, sizeof(keyStates));
	std::memcpy(prevMouseButtonStates, mouseButtonStates, sizeof(mouseButtonStates));
}

bool InputManager::isKeyPressed(int key)
{
	if (key < 0 || key >= GLFW_KEY_LAST) { return false; }
	return keyStates[key];
}

bool InputManager::isKeyJustPressed(int key) {
	if (key < 0 || key >= GLFW_KEY_LAST) { return false; };
	return ( keyStates[key] && !prevKeyStates[key] );
}

bool InputManager::isMouseButtonPressed(int button)
{
	if (button < 0 || button >= GLFW_MOUSE_BUTTON_LAST) { return false; }
	return mouseButtonStates[button];
}

glm::vec2 InputManager::getMousePosition()
{
	return glm::vec2(mouseX, mouseY);
}

glm::vec2 InputManager::getMouseDelta()
{
	float deltaX = static_cast<float>(mouseX - lastMouseX);
	float deltaY = static_cast<float>(lastMouseY - mouseY);

	lastMouseX = mouseX;
	lastMouseY = mouseY;

	return glm::vec2(deltaX * sensitivity.x, deltaY * sensitivity.y);
}

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key < 0 || key >= 1024) { return; }

	if (action == GLFW_PRESS) {
		keyStates[key] = true;
	}
	else if (action == GLFW_RELEASE) {
		keyStates[key] = false;
	}
}

void InputManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button < 0 || button >= 8) { return; }

	if (action == GLFW_PRESS) {
		mouseButtonStates[button] = true;
	}
	else if (action == GLFW_RELEASE) {
		mouseButtonStates[button] = false;
	}
}

void InputManager::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	mouseX = xpos;
	mouseY = ypos;
}

void InputManager::setMouseSensitivity(float sensitivityX, float sensitivityY) {
	sensitivity = glm::vec2(sensitivityX, sensitivityY);
}

glm::vec2 InputManager::getMouseSensitivity() {
	return sensitivity;
}