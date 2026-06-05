#pragma once
#include <glm/glm.hpp>

#include <GLFW/glfw3.h>

class InputManager
{
private:
	// Tablice do przechowywania stanów klawiszy i przycisków myszy. Indeks odpowiada kodowi klawisza lub przycisku
	static bool keyStates[GLFW_KEY_LAST];
	static bool prevKeyStates[GLFW_KEY_LAST];
	static bool mouseButtonStates[GLFW_MOUSE_BUTTON_LAST];
	static bool prevMouseButtonStates[GLFW_MOUSE_BUTTON_LAST];

	// Zmienne do przechowywania aktualnej i poprzedniej pozycji myszy co pozwala na obliczenie delty ruchu myszy
	static double mouseX;
	static double mouseY;
	static double lastMouseX;
	static double lastMouseY;

	static glm::vec2 mouseDelta;
	static glm::vec2 sensitivity;

public:
	//Metoda do aktualizacji stanów klawiszy poprzedniej klatki powinna być wywołana w pętli głównej
	static void updateKeyStates();

	// Metoda do aktualizacji stanów klawiszy i przycisków myszy powinna być wywoływana w głównej pętli gry po przetworzeniu zdarzeń!
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
	static void charCallback(GLFWwindow* window, unsigned int code);
	static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);

	// Metoda do sprawdzania stanu klawiszy
	static bool isKeyPressed(int key);
	//Metoda sprawdzająca czy w obecnej klatce status klawisza się zmienił
	static bool isKeyJustPressed(int key);
	// Metoda do sprawdzania stanu przycisków myszy
	static bool isMouseButtonPressed(int key);

	// Metoda do sprawdzania pozji myszy
	static glm::vec2 getMousePosition();

	//metda obliczająca delte ruchu myszy od ostatniej aktualizacji
	static glm::vec2 getMouseDelta();

	// Metoda do ustawiania czułości myszy która będzie używana do skalowania delty ruchu myszy co pozwala na dostosowanie szybkości obracania kamery
	static void setMouseSensitivity(float sensitivityX, float sensitivityY);
	// Metoda do pobierania aktualnej czułości myszy
	static glm::vec2 getMouseSensitivity();
};

