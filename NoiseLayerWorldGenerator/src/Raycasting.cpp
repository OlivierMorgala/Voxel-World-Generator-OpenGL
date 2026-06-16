#include "Raycasting.h"

// Konstruktor klasy Raycast inicjalizacja podstawowych zmiennych
Raycast::Raycast(float maxDistance, World* World, Shader* Shader) : maxDistance(maxDistance), world(World), shader(Shader), lastHitTime(std::chrono::steady_clock::now()) {}

// Metoda GetWorld: Zwraca wskaznik do obiektu World
World* Raycast::GetWorld()
{
	return world;
}


// Metoda RaycastDDA: Serce raycastu odpowiada za poprawne dzialanie calego raycastu
void Raycast::RaycastDDA(const glm::vec3& CameraPosition, const glm::vec3& CameraFront)
{

	/// System ograniczajacy szybkosc wykonywania raycasta -> aktualny hitCooldown to 30ms, czyli raycast bedzie wstanie sie wykonac co 30ms. To ograniczenie nie jest widoczne, a zmniejszy naklad na procesor
	auto now = std::chrono::steady_clock::now();
	if (now - lastHitTime < hitCooldown)
	{
		return;
	}
	lastHitTime = now;

	float MaxDistance = maxDistance;
	/// Wspolrzedne w jakim bloku znajduje sie kamera
	int x = floor(CameraPosition.x);
	int y = floor(CameraPosition.y);
	int z = floor(CameraPosition.z);
	///
	int StepX{};
	int StepY{};
	int StepZ{};

	/// delta ile dlugosci promienia miesci siê w jednym bloku
	float DeltaX = (CameraFront.x ==  0) ? 1e30f : std::abs(1.0f / CameraFront.x);   // odleglosc o ile blokow musi przesunac sie promien aby przesunac sie o jeden blok w osi x
	float DeltaY = (CameraFront.y == 0) ? 1e30f : std::abs(1.0f / CameraFront.y);// odleglosc o ile blokow musi przesunac sie promien aby przesunac sie o jeden blok w osi y
	float DeltaZ = (CameraFront.z == 0) ? 1e30f : std::abs(1.0f / CameraFront.z); // odleglosc o ile blokow musi przesunac sie promien aby przesunac sie o jeden blok w osi z

	// SIDEDIST -> to odlegosc punktu w ktorym znajduje aktualnie znajduje sie kamera do najblizszego boku bloku. (Inaczej to dlugosc promienia od "oczu" kamery do najblizszej sciany)
	float SideDistX{};
	float SideDistY{};
	float SideDistZ{};
	
	// Sidedist a delta: 
	// SideDist to dokladna odleglosc promienia do najblizszej sciany jakiegos bloku 
	// Delta to stala wartosc jak dlugi musi byc promien zeby przejsc jeden blok

	//// w zaleznosci gdzie patrzy sie kamera zmieniamy zmienne
	if (CameraFront.x < 0)
	{
		// teraz gdy patrzy sie w lewo
		StepX = -1;
		SideDistX = (CameraPosition.x - x) * DeltaX;
	}
	else
	{
		// teraz gdy patrzy sie w prawo
		StepX = 1;
		SideDistX = (x + 1.0f - CameraPosition.x) * DeltaX;
	}

	if (CameraFront.y < 0)
	{
		// teraz gdy patrzy d dol
		StepY = -1;
		SideDistY = (CameraPosition.y - y) * DeltaY;
	}
	else
	{
		StepY = 1;
		// teraz gdy patrzy do gory

		SideDistY = (y + 1.0f - CameraPosition.y) * DeltaY;
	}

	if (CameraFront.z < 0)
	{
		// teraz gdy patrzy do tyl
		StepZ = -1;
		SideDistZ = (CameraPosition.z - z) * DeltaZ;
	}
	else
	{
		// teraz gdy patrzy prosto
		StepZ = 1;
		SideDistZ = (z + 1.0f - CameraPosition.z) * DeltaZ;
	}

	float distance{};

	// Przed Petla ustawiamy BlockHit na false oraz wysylamy informacje do shadera o wyzerowanym stanie trafienia raycasta
	BlockHit = false;
	shader->setValue("BlockHasHit", 0);

	// Glowna Petla Raycasta: Dziala dopoki dlugosc promienia nie przekroczy dopuszczalnej odleglosci
	while (distance <= MaxDistance)
	{
		// Sprawdzam ktory SideDist jest najmniejsze i posylamy w tamtym kierunku raycast
		// Dodatkowo zmieniam parametry:
		// Dodaje do koordynatow bloku (x,y,z) Step, ktory w zaleznosci od pozycji frontu kamery ma wartosc albo -1 albo 1
		// Do SideDist dodaje Delta
		// Aktualizuje distance
		if (SideDistX <= SideDistY && SideDistX <= SideDistZ)
		{
			x += StepX;
			SideDistX += DeltaX;
			distance = SideDistX - DeltaX;
		}
		else if (SideDistY <= SideDistX && SideDistY <= SideDistZ)
		{
			y += StepY;
			SideDistY += DeltaY;
			distance = SideDistY - DeltaY;
		}
		else
		{
			z += StepZ;
			SideDistZ += DeltaZ;
			distance = SideDistZ - DeltaZ;
		}

		// Sprawdzenie jesli nasza dlugosc raycasta przekroczyla maksymalnie dopuszczana dlugosc to przerywamy petle
		if (distance > MaxDistance)
			break;

		// Pobieramy ID aktualnego bloku
		BlockID currentBlockID = world->getBlock(x, y, z);

		// Sprawdzamy czy Raycast trafil cos innego niz powietrze
		if (currentBlockID != 0) 
		{
			// Ustawiamy HitBlockPosition i wysylamy informacje do shadera aby podswietlil wybrany blok
			HitBlockPosition = { x,y,z };
			shader->setValueVec3("blockHitPosition", HitBlockPosition);
			shader->setValue("BlockHasHit", 1.0f);
			BlockHit = true;
			break;

		}


	}


}