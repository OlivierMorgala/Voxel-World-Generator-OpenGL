#pragma once
#include <glm/glm.hpp>
#include "managers/InputManager.h"

class Input
{
public:
	static bool isKeyPressed(int key);
	static bool isKeyJustPressed(int key);
	static bool isMouseButtonPressed(int button);
	static glm::vec2 getMousePosition();

	static glm::vec2 getMouseDelta();
};

