#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#include "world/WorldConfig.h"

// Struktura Plaszczyzn -> Ktora przyda sie w Camera Frustum
struct Plane {
	glm::vec3 normal;
	float distance;

	void normalizeData() {
		float mag = glm::length(normal);
		normal /= mag;
		distance /= mag;
	}
};

// Struktura Frustum -> Frustum zawiera obiekty Plane
struct Frustum {
	Plane far;
	Plane near;
	Plane top;
	Plane bottom;
	Plane right;
	Plane left;
};

struct AA_BoundingBox {
	glm::vec3 max;
	glm::vec3 min;
};


// Klasa Camera: Odpowiada za calosc kamery
class Camera
{
private:
	// Kąty Yaw i Pitch określają orientację kamery w przestrzeni. Yaw kontroluje obrót wokół osi Y, a Pitch kontroluje obrót wokół osi X.
	float yaw = -90.0f;
	float pitch = 0.0f;

	// Aktualizuje wektory right i up na podstawie aktualnej pozycji i orientacji kamery
	void updateCameraVectors();

public:
	// Enum do reprezentowania kierunków ruchu kamery
	enum CameraMovement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		UP,
		DOWN
	};

	glm::vec3 position; // Pozycja kamery w przestrzeni świata
	glm::vec3 front; // Kierunek, w którym kamera jest skierowana
	glm::vec3 up; // Wektor "gory" kamery, uzywany do okreslenia orientacji
	glm::vec3 right; // Wektor "prawo" kamery, prostopadły do front i up

	glm::vec3 worldUp; // Wektor "gory" swiata, używany do obliczania orientacji kamery

	float movementSpeed;

	// Konstruktor inicjalizujący kamerę na określonej pozycji.
	Camera(glm::vec3 startPosition = glm::vec3(0.0f, 0.0f, 3.0f));

	// Metoda do uzyskania macierzy widoku kamery ktora bedzie uzywana w shaderze
	glm::mat4 getViewMatrix() const;

	// Metoda do uzyskania macierzy projekcji kamery która będzie używana w shaderze.
	glm::mat4 getProjectionMatrix(float aspectRatio) const;

	// Metoda do przetwarzania wejścia z klawiatury i aktualizacji pozycji kamery
	void processKeyboardInput(CameraMovement direction, float deltaTime);

	// Metoda do przetwarzania ruchu myszy i aktualizacji orientacji kamery
	void processMouseMovement(float xoffset, float yoffset);


	//Frustum------------------------------------------------------------

	Frustum getFrustum(float aspectRatio) const;

	bool isAABoundingBoxVisible(const Frustum& frustum, const AA_BoundingBox& aabb) const;
};

