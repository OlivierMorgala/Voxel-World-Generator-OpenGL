#include "scenes\WorldGeneratorScene.h"
#include "scenes\LoadingScene.h"
#include "managers\SceneManager.h"

#include "world\TerrainPipeline.h"
#include "world\generationAlgorithms\PerlinNoise2D.h"
#include "world\generationAlgorithms\FlatFill.h"

// Konstruktor klasy sceny generatora świata
// Przyjmuje wektor warstw terenu (TerrainLayer) przez wartość i przenosi go (std::move) do pola klasy presetLayers
WorldGeneratorScene::WorldGeneratorScene(std::vector<TerrainLayer> initialLayers) {
    presetLayers = std::move(initialLayers);
}


// Metoda wywoływana automatycznie w momencie wejścia/załadowania sceny do menedżera scen
void WorldGeneratorScene::onEnter()
{
    std::cout << "+[Scene] LOADED: WorldGeneratorScene" << std::endl;

    // Jeśli okno aplikacji istnieje, domyślnie ukrywamy i blokujemy kursor myszy wewnątrz okna (tryb FPP)
    if (window) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    // Inicjalizacja kamery na pozycji (0, 0, 1) - tymczasowa wartość później bedzie trzeba ja ustalać wzgledem wytworzonego terenu
    camera = std::make_unique<Camera>(glm::vec3(8.0f, 40.0f, 8.0f));

    //TYMCZASOWO
    // Inicjalizacja statycznej bazy danych o blokach (ładowanie typów, tekstur itp.)
    BlockDatabase::init();
    //---

    // Kompilacja i załadowanie głównego programu cieniującego (Shader) z plików wierzchołków i fragmentów
    mainShader = std::make_unique<Shader>("shaders/main.vert", "shaders/main.frag");

    // Alokacja obiektu odpowiedzialnego za algorytmy generowania i przetwarzania warstw terenu
    worldTerrainGenerator = std::make_unique<WorldTerrainGenerator>();


    //Dodawanie warst przekazanych z kostruktorze
    // Jeśli przesłano gotowe warstwy terenu w konstruktorze, przepisujemy je do generatora terenu
    if (!presetLayers.empty()) {
        for (TerrainLayer& layer : presetLayers) {
            worldTerrainGenerator->generationLayers.push_back(std::move(layer));
        }
        presetLayers.clear(); // Czyszczenie kontenera tymczasowego po przeniesieniu zasobów
    }
    else {
        // W przypadku braku warstw startowych, tworzona jest domyślna płaska warstwa "Base Fill" wypisująca teren do wysokości 5
        TerrainLayer baseLayer("Base Fill", 0, 5, 2, std::make_unique<FlatFill>());
        baseLayer.blendMode = BlendMode::NORMAL;
        worldTerrainGenerator->generationLayers.push_back(std::move(baseLayer));
    }


    // Inicjalizacja głównego obiektu zarządzającego światem (siatki chunków)
    world = std::make_unique<World>();
    // Powiązanie wskaźników do kamery oraz generatora terenu ze strukturą świata
    world->setCamera(camera.get());
    world->setTerrainGenerator(worldTerrainGenerator.get());

    // Alokacja systemów renderujących: głównego renderera świata oraz renderera pomocniczego (debugowanie siatek/linii)
    worldRenderer = std::make_unique<WorldRenderer>();
    debugRenderer = std::make_unique<DebugRenderer>();

    // Inicjalizacja dedykowanego interfejsu graficznego ImGui dla generatora świata
    worldGenUI = std::make_unique<WorldGeneratorUI>(worldTerrainGenerator.get(), world.get());

    // Raycast - utworzenie promienia o maksymalnym zasięgu 60 jednostek, służącego do wykrywania bloków przed kamerą
    raycast = std::make_unique<Raycast>(60, world.get(), mainShader.get());

    // KLASA ODPOWIADAJACA ZA STAWIANIE I NISZCZENIE BLOKOW
    // Przekazanie promienia raycast do obiektu modyfikującego strukturę bloków świata
    blockPlaceDestroy = std::make_unique<BlockPlaceDestroy>(raycast.get());

    // Pierwsze, wstępne wywołanie generowania struktury świata
    world->regenerateWorld();

    // Pchnięcie na stos menedżera sceny ładowania (LoadingScene), która wyświetla ekran ładowania podczas budowania chunków
    SceneManager::getInstance().pushScene(std::make_unique<LoadingScene>(world.get()));
}


// Metoda wywoływana automatycznie podczas opuszczania lub niszczenia sceny
void WorldGeneratorScene::onExit()
{
    // Jeśli okno istnieje, przywracamy standardowy, widoczny i wolny kursor myszy
    if (window) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    std::cout << "-[Scene] UNLOADED: WorldGeneratorScene" << std::endl;
}


// Metoda odpowiedzialna za aktualizację logiki gry w każdej klatce (fizyka, interakcje, wejście)
void WorldGeneratorScene::onUpdate(float deltaTime)
{
    // Warunek bezpieczeństwa - jeśli kamera lub okno nie zainicjalizowały się, przerywamy pętlę aktualizacji
    if (!camera || !window) { return; }

    // Obsługa poruszania się kamery/gracza (aktywne tylko wtedy, gdy menu edycji jest zamknięte i kursor jest zablokowany)
    if (!isCursorMode) {
        if (Input::isKeyPressed(GLFW_KEY_W)) camera->processKeyboardInput(Camera::FORWARD, deltaTime);
        if (Input::isKeyPressed(GLFW_KEY_S)) camera->processKeyboardInput(Camera::BACKWARD, deltaTime);
        if (Input::isKeyPressed(GLFW_KEY_A)) camera->processKeyboardInput(Camera::LEFT, deltaTime);
        if (Input::isKeyPressed(GLFW_KEY_D)) camera->processKeyboardInput(Camera::RIGHT, deltaTime);
        if (Input::isKeyPressed(GLFW_KEY_LEFT_SHIFT)) camera->processKeyboardInput(Camera::DOWN, deltaTime);
        if (Input::isKeyPressed(GLFW_KEY_SPACE)) camera->processKeyboardInput(Camera::UP, deltaTime);
    }

    // Przełączanie trybu kursora (Menu ImGui <-> Tryb Rozgrywki FPP) za pomocą klawisza ESCAPE
    if (Input::isKeyJustPressed(GLFW_KEY_ESCAPE)) {
        isCursorMode = !isCursorMode; // Negacja flagi stanu kursora
        // Zmiana trybu przechwytywania myszy w GLFW w zależności od aktualnego stanu flagi
        glfwSetInputMode(window, GLFW_CURSOR, isCursorMode ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

        // Modyfikacja flag ImGui, aby interfejs poprawnie reagował lub ignorował interakcje myszy
        if (isCursorMode) {
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse; // Włączenie obsługi myszy w ImGui
        }
        else {
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;  // Ignorowanie myszy przez ImGui (kontrola wraca do gry)
        }
    }

    // Jeśli kursor jest zablokowany w grze, pobieramy przesunięcie myszy (delta) i obracamy kamerą FPP
    if (!isCursorMode) {
        glm::vec2 delta = Input::getMouseDelta();
        camera->processMouseMovement(delta.x, delta.y);
    }

    // Aktualizacja wątków, ładowania obszarów wokół kamery i logiki wewnątrz struktury świata
    if (world) {
        world->updateWorld();
    }

    // Wykonanie algorytmu DDA (Digital Differential Analysis) dla promienia w celu znalezienia trafionego bloku
    if (raycast)
    {
        raycast->RaycastDDA(camera->position, camera->front);
    }

    // Jeśli promień trafił w blok, a gracz wcisnął prawy przycisk myszy lub klawisz E, wywoływane jest niszczenie tego bloku
    if (blockPlaceDestroy && raycast->BlockHit && (Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_2) || Input::isKeyJustPressed(GLFW_KEY_E)))
    {
        blockPlaceDestroy->DestroyBlock(raycast->HitBlockPosition);
    }
}


// Główna metoda renderowania grafiki 3D za pomocą OpenGL (wywoływana co klatkę)
void WorldGeneratorScene::render()
{
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Opcjonalny tryb renderowania siatkowego (wireframe) do celów testowych

    // Bezpiecznik - jeśli kluczowe komponenty renderowania nie istnieją, pomijamy tę klatkę rysowania
    if (!world || !worldRenderer || !debugRenderer || !mainShader) return;

    // Pobranie macierzy widoku (View Matrix) z kamery i przesłanie jej do pamięci shadera pod zmienną uniform "view"
    glm::mat4 view = camera->getViewMatrix();
    mainShader->setMatrix4("view", view);


    if (window) {
        int width;
        int height;
        // Pobranie aktualnego rzeczywistego rozmiaru bufora ramki okna (przydatne przy zmianie rozmiaru okna)
        glfwGetFramebufferSize(window, &width, &height);

        if (height == 0) { height = 1; } // Zabezpieczenie przed dzieleniem przez zero
        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

        // Ustawienie wymiarów okna renderowania (Viewport OpenGL)
        glViewport(0, 0, width, height);

        // Zmiana koloru czyszczenia ekranu (glClearColor) w zależności od tego, czy kamera znajduje się pod wodą
        if (worldRenderer->isCameraUnderwater) {
            // Kolor podwodny przemnożony przez współczynnik przyciemnienia, imitujący głębię głębinową
            glm::vec3 depthColor = worldRenderer->underwaterColor * 0.4f;
            glClearColor(depthColor.r, depthColor.g, depthColor.b, 1.0f);
        }
        else {
            // Standardowy, ciemnoszary kolor tła dla czystego nieba/świata
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        }

        // Wyczyszczenie bufora koloru oraz bufora głębokości (Z-Buffer) przed nowym rysowaniem
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST); // Włączenie testowania głębi (ukrywanie zasłoniętych powierzchni)


        // Jeśli świat przetworzył zapytania i przeszedł w stan PLAYING, generujemy i rysujemy geometrię
        if (world->getCurrentState() == WorldState::PLAYING) {
            // Renderowanie bloków i terenu świata
            worldRenderer->render(*world, *camera, *mainShader, aspectRatio);

            // Jeśli w konfiguracji włączono rysowanie granic kolumn chunków, debugRenderer nanosi je na ekran
            if (config.showChunkColumnsBorder) {
                debugRenderer->renderChunkBorders(worldRenderer.get()->getVisibleColumns(), *camera, aspectRatio);
            }
        }

    }
}


// Metoda dedykowana do rysowania elementów interfejsu użytkownika (2D HUD, menu) przy pomocy biblioteki ImGui
void WorldGeneratorScene::onImGuiRender()
{
    // Konfiguracja flag okien ImGui służących do budowy przezroczystych, zablokowanych paneli HUD (bez ramek, bez możliwości przesuwania)
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoMove;


    //----------------------Filtr na kamere w bloku przeźroczystym
    // Tworzenie pełnoekranowego kolorowego filtru nakładki (overlay), gdy kamera wejdzie pod wodę (np. w blok przezroczysty)
    if (config.isTransparentBlockCameraFilterEnabled) {
        if (worldRenderer && worldRenderer->isCameraUnderwater) {
            ImVec2 screenSize = ImGui::GetIO().DisplaySize; // Pobranie rozmiaru ekranu aplikacji

            // Konwersja składowych koloru float (0.0 - 1.0) na format całkowitoliczbowy RGB (0 - 255)
            int r = static_cast<int>(worldRenderer->underwaterColor.r * 255.0f);
            int g = static_cast<int>(worldRenderer->underwaterColor.g * 255.0f);
            int b = static_cast<int>(worldRenderer->underwaterColor.b * 255.0f);

            // Utworzenie koloru spakowanego 32-bitowego z przezroczystością alpha ustawioną na 140
            ImU32 filterColor = IM_COL32(r, g, b, 140);
            // Wyrenderowanie prostokąta na całej powierzchni okna w najniższej warstwie tła rysowania ImGui (BackgroundDrawList)
            ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0.0f, 0.0f), screenSize, filterColor);
        }
    }



    //-----------------------COORDS AND FPS MENU
    // Panel HUD wyświetlający aktualną pozycję XYZ kamery oraz liczbę klatek na sekundę (FPS) w lewym górnym rogu ekranu
    if (config.isCoordsHudEnabled) {

        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always); // Pozycja sztywna (10, 10) pikseli
        ImGui::SetNextWindowBgAlpha(0.3f); // Wysoki stopień przezroczystości tła okna panelu

        if (ImGui::Begin("CORDHUD", nullptr, window_flags)) {
            if (camera) {
                // Wyświetlanie współrzędnych sformatowanych do dwóch miejsc po przecinku
                ImGui::Text("X: %.2f", camera.get()->position.x);
                ImGui::SameLine();
                ImGui::Text(" | ");
                ImGui::SameLine();
                ImGui::Text("Y: %.2f", camera.get()->position.y);
                ImGui::SameLine();
                ImGui::Text(" | ");
                ImGui::SameLine();
                ImGui::Text("Z: %.2f", camera.get()->position.z);
                // Pobranie i wyświetlenie uśrednionego licznika klatek z biblioteki ImGui
                ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            }
            ImGui::Separator();
        }
        ImGui::End();
    }



    //-----------------------CHUNK DATA MENU
    // Panel HUD wyświetlający statystyki dotyczące zarządzania pamięcią chunków, umiejscowiony w lewym dolnym rogu ekranu
    if (config.isChunkHudEnabled) {
        ImVec2 screenSize = ImGui::GetIO().DisplaySize;

        // Ustawienie pozycji na dole ekranu z użyciem punktu zakotwiczenia (Pivot) w lewym dolnym narożniku okna panelu (0.0f, 1.0f)
        ImGui::SetNextWindowPos(ImVec2(10.0f, screenSize.y - 10.0f), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
        ImGui::SetNextWindowBgAlpha(0.3f);

        if (ImGui::Begin("RENDER_STATS_HUD", nullptr, window_flags)) {

            size_t visibleColsCount = 0;
            if (worldRenderer) {
                // Pobranie liczby aktualnie renderowanych kolumn chunków (widocznych w stożku widoku kamery - Frustum Culling)
                visibleColsCount = worldRenderer->getVisibleColumns().size();
            }

            size_t totalColsCount = 0;
            if (world) {
                // Pobranie całkowitej liczby zaalokowanych kolumn chunków przechowywanych aktualnie w pamięci RAM/VRAM
                totalColsCount = world->getLoadedChunkColumnsCount();
            }

            int heightInChunks = config.worldHeightInChunks; // Wysokość świata wyrażona w ilości pod-chunków pionowych

            ImGui::SetWindowFontScale(1.3f);
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "CHUNKS:");
            ImGui::SetWindowFontScale(1.0f);

            ImGui::Separator();

            // Statystyki podsystemu rysowania (widoczne)
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "RENDERED (Visible)");
            ImGui::Text("Columns : %zu", visibleColsCount);
            ImGui::Text("Chunks  : %zu", visibleColsCount * heightInChunks);

            ImGui::Spacing();
            ImGui::Separator();

            // Ogólny stan zużycia pamięci sceny
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "TOTAL IN MEMORY");
            ImGui::Text("Columns : %zu", totalColsCount);
            ImGui::Text("Chunks  : %zu", totalColsCount * heightInChunks);
        }
        ImGui::End();
    }



    //-----------------------PASEK AKTYWNEGO MENU
    // Panel informacyjny (podpowiedź tekstowa) zawieszony centralnie na samej górze ekranu
    ImGuiWindowFlags hintFlags = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoMove;

    ImVec2 screenSize = ImGui::GetIO().DisplaySize;

    // Centrowanie poziome na ekranie (screenSize.x * 0.5f) z przesunięciem 5 pikseli od góry, pivot wyśrodkowany (0.5f, 0.0f)
    ImGui::SetNextWindowPos(ImVec2(screenSize.x * 0.5f, 5.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.65f);

    // Dynamiczna modyfikacja stylów ImGui - dodanie wyraźnej ramki wokół paska podpowiedzi
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.9f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);

    if (ImGui::Begin("EscHintOverlay", nullptr, hintFlags)) {

        ImGui::SetWindowFontScale(1.3f);

        // Wyświetlanie komunikatu dopasowanego kolorystycznie do trybu edycji świata (Czerwony / Zielony)
        if (isCursorMode) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Press [ESC] to exit world edit mode");
        }
        else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Press [ESC] to enter world edit mode");
        }

    }
    ImGui::End();

    // Przywrócenie pierwotnych stylów ImGui ze stosu globalnego (zdjęcie wprowadzonych modyfikacji ramki)
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();



    //-----------------------CELOWNIK
    // Rysowanie klasycznego celownika (krzyżyka) na środku ekranu, widocznego wyłącznie w trybie eksploracji/rozgrywki FPP
    if (config.isCrosshairEnabled && !isCursorMode) {
        ImVec2 center = ImVec2(screenSize.x * 0.5f, screenSize.y * 0.5f); // Punkt środka ekranu

        // Pobranie najwyższej warstwy rysowania ImGui nanoszonej na sam wierzch gotowego obrazu (ForegroundDrawList)
        ImDrawList* drawList = ImGui::GetForegroundDrawList();

        float dotSize = 4.0f; // Grubość ramion celownika
        float gap = 2.0f;     // Przerwa centralna celownika
        ImU32 color = IM_COL32(30, 255, 30, 255); // Kolor celownika (zielony)

        // Rysowanie 4 prostokątów tworzących symetryczne ramiona krzyżyka (lewe, prawe, górne, dolne)
        drawList->AddRectFilled(ImVec2(center.x - gap - dotSize, center.y - dotSize * 0.5f), ImVec2(center.x - gap, center.y + dotSize * 0.5f), color);
        drawList->AddRectFilled(ImVec2(center.x + gap, center.y - dotSize * 0.5f), ImVec2(center.x + gap + dotSize, center.y + dotSize * 0.5f), color);
        drawList->AddRectFilled(ImVec2(center.x - dotSize * 0.5f, center.y - gap - dotSize), ImVec2(center.x + dotSize * 0.5f, center.y - gap), color);
        drawList->AddRectFilled(ImVec2(center.x - dotSize * 0.5f, center.y + gap), ImVec2(center.x + dotSize * 0.5f, center.y + gap + dotSize), color);
    }



    //-----------------------GŁÓNE MENU
    // Wywołanie rysowania całego zaawansowanego okna menu edycji warstw (WorldGeneratorUI), przekazując aktualny stan kursora
    if (worldGenUI) {
        worldGenUI->renderImGui(isCursorMode);
    }
}