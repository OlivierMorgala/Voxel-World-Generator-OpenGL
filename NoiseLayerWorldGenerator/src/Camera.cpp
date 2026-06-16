#include "Camera.h"

// Konstruktor inicjalizuje podstawowe parametry camery
Camera::Camera(glm::vec3 startPosition) :
	position(startPosition),
	front(glm::vec3(0.0f, 0.0f, -1.0f)),
	worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
	movementSpeed(20.0f)
{
	updateCameraVectors();
}

// Metoda getViewMatrix: Metoda zwraca macierz widoku
glm::mat4 Camera::getViewMatrix() const
{
	return glm::lookAt(position, position + front, up);
}

// Metoda getProjectionMatrix: Metoda zwraca macierz projekcji okreslajaca fov, proporcje oraz zasieg renderowania
glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const
{
	return glm::perspective(glm::radians(config.fov), aspectRatio, config.viewBegin, config.viewDistance);
}

// Metoda procesKeyboardInput: Metoda obsluguje ruch kamery przy pomocy klawiatury
void Camera::processKeyboardInput(CameraMovement direction, float deltaTime)
{
	// Predkosc ruchu jest skalowana przez deltaTime, aby zapewnic plynny ruch niezaleznie od liczby klatek na sekunde
	float velocity = config.cameraSpeed * deltaTime;

	// Aktualizujemy pozycje kamery na podstawie kierunku ruchu
	if (direction == FORWARD)  position += front * velocity;
	if (direction == BACKWARD) position -= front * velocity;
	if (direction == LEFT)     position -= right * velocity;
	if (direction == RIGHT)    position += right * velocity;
	if (direction == UP)       position += worldUp * velocity;
	if (direction == DOWN)     position -= worldUp * velocity;
}

// Metoda processMouseMovement: Metoda odpowiada za obsluge obracania kamery za pomoca myszy
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

// Metoda updateCameraVectors: Metoda odpowiada za przeliczenie wektorow Front, Right, Up
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

//Frustum------------------------------------------------------------

// Metoda getFrustum: Metoda buduje wirtualne sciany naszego stozka widzenia 
Frustum Camera::getFrustum(float aspectRatio) const 
{
	Frustum frustum;

	
	glm::mat4 proj = getProjectionMatrix(aspectRatio);
	glm::mat4 view = getViewMatrix();
	
	glm::mat4 viewProjectionMatrix = proj * view;

	frustum.far.normal.x = viewProjectionMatrix[0][3] - viewProjectionMatrix[0][2];
	frustum.far.normal.y = viewProjectionMatrix[1][3] - viewProjectionMatrix[1][2];
	frustum.far.normal.z = viewProjectionMatrix[2][3] - viewProjectionMatrix[2][2];
	frustum.far.distance = viewProjectionMatrix[3][3] - viewProjectionMatrix[3][2];

	frustum.near.normal.x = viewProjectionMatrix[0][3] + viewProjectionMatrix[0][2];
	frustum.near.normal.y = viewProjectionMatrix[1][3] + viewProjectionMatrix[1][2];
	frustum.near.normal.z = viewProjectionMatrix[2][3] + viewProjectionMatrix[2][2];
	frustum.near.distance = viewProjectionMatrix[3][3] + viewProjectionMatrix[3][2];

	frustum.right.normal.x = viewProjectionMatrix[0][3] - viewProjectionMatrix[0][0];
	frustum.right.normal.y = viewProjectionMatrix[1][3] - viewProjectionMatrix[1][0];
	frustum.right.normal.z = viewProjectionMatrix[2][3] - viewProjectionMatrix[2][0];
	frustum.right.distance = viewProjectionMatrix[3][3] - viewProjectionMatrix[3][0];

	frustum.left.normal.x = viewProjectionMatrix[0][3] + viewProjectionMatrix[0][0];
	frustum.left.normal.y = viewProjectionMatrix[1][3] + viewProjectionMatrix[1][0];
	frustum.left.normal.z = viewProjectionMatrix[2][3] + viewProjectionMatrix[2][0];
	frustum.left.distance = viewProjectionMatrix[3][3] + viewProjectionMatrix[3][0];

	frustum.top.normal.x = viewProjectionMatrix[0][3] - viewProjectionMatrix[0][1];
	frustum.top.normal.y = viewProjectionMatrix[1][3] - viewProjectionMatrix[1][1];
	frustum.top.normal.z = viewProjectionMatrix[2][3] - viewProjectionMatrix[2][1];
	frustum.top.distance = viewProjectionMatrix[3][3] - viewProjectionMatrix[3][1];

	frustum.bottom.normal.x = viewProjectionMatrix[0][3] + viewProjectionMatrix[0][1];
	frustum.bottom.normal.y = viewProjectionMatrix[1][3] + viewProjectionMatrix[1][1];
	frustum.bottom.normal.z = viewProjectionMatrix[2][3] + viewProjectionMatrix[2][1];
	frustum.bottom.distance = viewProjectionMatrix[3][3] + viewProjectionMatrix[3][1];

	frustum.far.normalizeData();
	frustum.near.normalizeData();
	frustum.right.normalizeData();
	frustum.left.normalizeData();
	frustum.top.normalizeData();
	frustum.bottom.normalizeData();

	return frustum;
}

// Metoda isAABoundingBoxVisible: Metoda sprawdza czy dane trojwymiarowy blok/pudelko np.Chunk jest w ogole na ekranie - czyli czy mozemy go zobaczyc
bool Camera::isAABoundingBoxVisible(const Frustum& frustum, const AA_BoundingBox& aabb) const
{
	// Uzywamy naszych 6 scian aby uzyc ich w petli
	const Plane* planes[6] = { &frustum.far, &frustum.near, &frustum.top, &frustum.bottom, &frustum.right, &frustum.left };

	for (int i = 0; i < 6; i++) {
		// Szukamy tego rogu trojwymiarowego pudelka np.Chunka, ktory jest najbardziej wysuniety w strone aktualnej sciany
		glm::vec3 farthestVertex = aabb.min;
		if (planes[i]->normal.x >= 0) { farthestVertex.x = aabb.max.x; }
		if (planes[i]->normal.y >= 0) { farthestVertex.y = aabb.max.y; }
		if (planes[i]->normal.z >= 0) { farthestVertex.z = aabb.max.z; }

		// Jesli najbardziej wysuniety rog pudelka schowal sie za nasza sciana Frustruma
		// to na 100% mozemy stwierdzic, ze pudelko nie znajduje sie w polu widzenia i gracz go nie widzi
		if (glm::dot(planes[i]->normal, farthestVertex) + planes[i]->distance < 0) {
			return false; // Ustawiamy false
		}

	}

	return true; // Jesli "pudelko" znajduje sie choc troche w polu widzenia to return true
}