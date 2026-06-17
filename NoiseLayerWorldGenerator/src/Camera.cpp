#include "Camera.h"

Camera::Camera(glm::vec3 startPosition) :
	position(startPosition),
	front(glm::vec3(0.0f, 0.0f, -1.0f)),
	worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
	movementSpeed(20.0f)
{
	updateCameraVectors();
}

//Metoda zwarcająca macierz widoku (która służy do przsuwania świata w kierunku przciwnym do kamery co symuluje poruszenie się kamery)
glm::mat4 Camera::getViewMatrix() const
{
	return glm::lookAt(position, position + front, up);
}

//Metoda zwracająca macierz projekcji (która słuzy do symulacji prespektywy np. dalsze obeikty stają się mniejsze)
glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const
{
	return glm::perspective(glm::radians(config.fov), aspectRatio, config.viewBegin, config.viewDistance);
}

void Camera::processKeyboardInput(CameraMovement direction, float deltaTime)
{
	// Prędkość ruchu jest skalowana przez deltaTime, aby zapewnić płynny ruch niezależnie od liczby klatek na sekundę
	float velocity = config.cameraSpeed * deltaTime;

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

//Frustum------------------------------------------------------------

Frustum Camera::getFrustum(float aspectRatio) const 
{
	Frustum frustum;

	
	glm::mat4 proj = getProjectionMatrix(aspectRatio);
	glm::mat4 view = getViewMatrix();
	
	//Mnożymy macierz projekcji razy macierz widoku by otrzymać tzw macierz widoku projeckji
	glm::mat4 viewProjectionMatrix = proj * view;


	//Z otrzymanej macierzy jesteśmy w stanie wydobyć 6 płaszczyzn tworzących (frustum) figurę przypomniająca ścięty ostrosłup (Algorytm Gribba-Hartmanna przeczytać!)
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

	//Obliczanie wektorów normalnych płaszczyzn (vektor prostopadły do płaszczyzny wskzujący na środek ekranu)
	frustum.far.normalizeData();
	frustum.near.normalizeData();
	frustum.right.normalizeData();
	frustum.left.normalizeData();
	frustum.top.normalizeData();
	frustum.bottom.normalizeData();

	return frustum;
}


//(AABB - Axis-Aligned Bounding Box)
//Metoda sprawdza czy prostopadłościan będący naszą kolmną znajduje sie w zasiegu pola widzenia kamery
bool Camera::isAABoundingBoxVisible(const Frustum& frustum, const AA_BoundingBox& aabb) const
{
	
	const Plane* planes[6] = { &frustum.far, &frustum.near, &frustum.top, &frustum.bottom, &frustum.right, &frustum.left };

	//Testujemy AABB przeciwko każdej z 6 płaszczyzn kamery
	for (int i = 0; i < 6; i++) {

		//Zmiast sprawdzać każdy z 8 wierzchołków sprawdzamy tylko wierzchołek który jest najbliżej płaszczyzny
		//Max - prawo Min - Lewo  (jeśli wektor noramlny jest na + to patrzy w prawo)

		glm::vec3 farthestVertex = aabb.min; 
		if (planes[i]->normal.x >= 0) { farthestVertex.x = aabb.max.x; }  //Jeśli patrzymy w prawo to punkt będzie z prawej strony pudełka
		if (planes[i]->normal.y >= 0) { farthestVertex.y = aabb.max.y; }
		if (planes[i]->normal.z >= 0) { farthestVertex.z = aabb.max.z; }

		//Jeśli najbliższy wierzchołek jest za płaszczyzną tnącą mówi to że obiektu nie widać (glm::dot liczy odległość od płaszczyzny)
		if (glm::dot(planes[i]->normal, farthestVertex) + planes[i]->distance < 0) {
			return false;
		}

	}

	return true; //Jesli żadan płaszczyzna go nie odrzuciła rysujemy kolumne
}