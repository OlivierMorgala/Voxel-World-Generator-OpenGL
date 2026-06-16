#include "managers/InputManager.h"
#include <imgui_impl_glfw.h>

// Tablice stanów klawiszy dla aktualnej i poprzedniej klatki
bool InputManager::keyStates[GLFW_KEY_LAST] = { false };
bool InputManager::prevKeyStates[GLFW_KEY_LAST] = { false };
bool InputManager::mouseButtonStates[GLFW_MOUSE_BUTTON_LAST] = { false };
bool InputManager::prevMouseButtonStates[GLFW_MOUSE_BUTTON_LAST] = { false };

// Pozycja myszy i wartości delta (przesunięcie w klatce)
double InputManager::mouseX = 0.0;
double InputManager::mouseY = 0.0;
double InputManager::lastMouseX = 0.0;
double InputManager::lastMouseY = 0.0;
glm::vec2 InputManager::mouseDelta = glm::vec2(0.0f, 0.0f);
glm::vec2 InputManager::sensitivity = glm::vec2(0.05f, 0.05f);

// Aktualizacja stanów wywoływana co klatkę
void InputManager::updateKeyStates() {
	std::memcpy(prevKeyStates, keyStates, sizeof(keyStates));
	std::memcpy(prevMouseButtonStates, mouseButtonStates, sizeof(mouseButtonStates));

	// Obliczanie przesunięcia (delta) - Y jest odwrócone (ekran vs świat 3D)
	float deltaX = static_cast<float>(mouseX - lastMouseX);
	float deltaY = static_cast<float>(lastMouseY - mouseY);

	mouseDelta = glm::vec2(deltaX * sensitivity.x, deltaY * sensitivity.y);

	lastMouseX = mouseX;
	lastMouseY = mouseY;
}

// Sprawdzenie, czy klawisz jest wciśnięty
bool InputManager::isKeyPressed(int key)
{
	if (key < 0 || key >= GLFW_KEY_LAST) { return false; }
	return keyStates[key];
}

// Sprawdzenie, czy klawisz został wciśnięty dokładnie w TEJ klatce
bool InputManager::isKeyJustPressed(int key) {
	if (key < 0 || key >= GLFW_KEY_LAST) { return false; };
	return (keyStates[key] && !prevKeyStates[key]);
}

// Sprawdzenie, czy kliknięto przycisk myszy
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
	return mouseDelta;
}

// Obsługa zdarzeń z GLFW z uwzględnieniem ImGui
void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

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
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

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
	ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

	mouseX = xpos;
	mouseY = ypos;
}

void InputManager::charCallback(GLFWwindow* window, unsigned int code) {
	ImGui_ImplGlfw_CharCallback(window, code);
}

void InputManager::scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
	ImGui_ImplGlfw_ScrollCallback(window, xOffset, yOffset);
}

void InputManager::setMouseSensitivity(float sensitivityX, float sensitivityY) {
	sensitivity = glm::vec2(sensitivityX, sensitivityY);
}

glm::vec2 InputManager::getMouseSensitivity() {
	return sensitivity;
}
