#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

#include "managers/InputManager.h"
#include "managers/SceneManager.h"
#include "managers/WindowManager.h"

#include "scenes/WorldGeneratorScene.h"

int main() { 
    
	//Inicjalizacja GLFW
	if (!glfwInit()) { return -1; }

	//Stworzenie okna
	GLFWwindow* window = glfwCreateWindow(1920, 1080, "World Generator", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//Ustawienie głównego okna w WindowManagerze
	WindowManager::getInstance().setMainWindow(window);

	//Inicjalizacja GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		return -1;
	}
	
	//Konfiguracja ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	//Ustawienie stylu ImGui na ciemny
	ImGui::StyleColorsDark();

	//Inicjalizacja backendów ImGui dla GLFW i OpenGL3
	ImGui_ImplGlfw_InitForOpenGL(window, true); 
	ImGui_ImplOpenGL3_Init("#version 330");

	//Ustawienie callbacków dla klawiatury i myszy które będą aktualizować stany klawiszy i przycisków myszy w InputManagerze co pozwoli na łatwe sprawdzanie tych stanów w logice gry
	glfwSetKeyCallback(window, InputManager::keyCallback);
	glfwSetMouseButtonCallback(window, InputManager::mouseButtonCallback);
	glfwSetCursorPosCallback(window, InputManager::cursorPositionCallback);	

	//Włączenie testu głębokości co pozwala na poprawne renderowanie obiektów w 3D z uwzględnieniem ich odległości od kamery
	glEnable(GL_DEPTH_TEST);
	
	//Włączenie cullingu co pozwala na pomijanie rysowania ścianek które są skierowane do tyłu względem kamery
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); 
	glFrontFace(GL_CCW); // Ustawienie kierunku wierzchołków przeciwnych do ruchu wskazówek zegara

	//Inicjalizaca sceny startowej programu
	SceneManager sceneManager;
	sceneManager.setScene(std::make_unique<WorldGeneratorScene>());

	float lastFrameTime = 0.0f;

	//Główna pętla
	while (!glfwWindowShouldClose(window)) {
		//Obliczanie czasu deltaTime dla aktualizacji logiki gry i animacji co pozwala na płynne działanie gry niezależnie od liczby klatek na sekundę
		float currentFrame = static_cast<float>(glfwGetTime());
		float deltaTime = currentFrame - lastFrameTime;
		lastFrameTime = currentFrame;

		//Pobranie danych wejściowych i przetworzenie zdarzeń systemowych takich jak zamknięcie okna, zmiana rozmiaru itp Ta funkcja jest kluczowa dla interakcji użytkownika z aplikacją i musi być wywoływana w każdej klatce
		glfwPollEvents();

		//Czyszczenie ekranu i przygotowanie do renderowania (czyszczenie bufora kolorów i bufora głębokości)
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Aktualizacja logiki gry i renderowanie sceny
		sceneManager.update(deltaTime);
		sceneManager.render();

		//Wyświetlanie renderowanej sceny na ekranie
		glfwSwapBuffers(window);
	}

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
   
   
}