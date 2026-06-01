#include "Camera.h"

Camera::Camera(glm::vec3 startPosition) :
	position(startPosition),
	front(glm::vec3(0.0f, 0.0f, -1.0f)),
	worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
	movementSpeed(20.0f)
{
	updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const
{
	return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const
{
	return glm::perspective(glm::radians(config.fov), aspectRatio, config.viewBegin, config.viewDistance);
}

void Camera::processKeyboardInput(Camera_Movement direction, float deltaTime)
{
	// Prędkość ruchu jest skalowana przez deltaTime, aby zapewnić płynny ruch niezależnie od liczby klatek na sekundę
	float velocity = movementSpeed * deltaTime;

	// Aktualizujemy pozycję kamery na podstawie kierunku ruchu
	if (direction == FORWARD)  position += front * velocity;
	if (direction == BACKWARD) position -= front * velocity;
	if (direction == LEFT)     position -= right * velocity;
	if (direction == RIGHT)    position += right * velocity;
	if (direction == UP)       position += worldUp * velocity;
	if (direction == DOWN)     position -= worldUp * velocity;
}

void Camera::processMouseMovement(float xoffset, float yoffset)
{
	// Aktualizujemy kąty Yaw(odchylenia - X) i Pitch(pochylenie - Y) na podstawie ruchu myszy
	yaw += xoffset;										
	pitch += yoffset;

	// Ograniczamy kąt Pitch, aby uniknąć efektu "przewrócenia" kamery, gdy patrzy prosto w górę lub w dół
	if (pitch > 89.0f) { pitch = 89.0f; }
	if (pitch < -89.0f) { pitch = -89.0f; }

	// Po aktualizacji kątów aktualizujemy wektory kamery aby zachować poprawną orientację
	updateCameraVectors();
}

void Camera::updateCameraVectors()
{
	//Obliczamy nowy wektor kierunku patrzenia kamery (front) na podstawie aktualnych kątów Yaw i Pitch. Używamy funkcji trygonometrycznych aby przekształcić kąty na wektor kierunku w przestrzeni 3D
	glm::vec3 newFront;
	newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	newFront.y = sin(glm::radians(pitch));
	newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	//Normalizujemy wektor front aby zapewnić że ma długość 1 co jest ważne dla poprawnych obliczeń ruchu i orientacji kamery
	front = glm::normalize(newFront);
	//Obliczamy iloczny wektorowy między front a worldUp aby uzyskać wektor right który daj nam nasze odchylenie w lewo/prawo. Normalizujemy go aby zapewnić że ma długość 1
	right = glm::normalize(glm::cross(front, worldUp));
	//Obliczamy iloczny wektorowy między right a front aby uzyskać wektor up który daj nam nasze odchylenie w górę/dół. Normalizujemy go aby zapewnić że ma długość 1
	up = glm::normalize(glm::cross(right, front));
}