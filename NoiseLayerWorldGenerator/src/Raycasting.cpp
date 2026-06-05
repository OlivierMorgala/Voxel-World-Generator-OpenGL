#include "Raycasting.h"

Raycast::Raycast(float maxDistance, World* World) : maxDistance(maxDistance), world(World), lastHitTime(std::chrono::steady_clock::now()) {}
void Raycast::RaycastDDA(const glm::vec3& CameraPosition, const glm::vec3& CameraFront)
{

	float MaxDistance = maxDistance;
	/// WSPOLRZEDNE W JAKIM BLOKU (X,Y,Z) ZNAJDUJE SIE KAMERA
	int x = floor(CameraPosition.x);
	int y = floor(CameraPosition.y);
	int z = floor(CameraPosition.z);
	int StepX{};
	int StepY{};
	int StepZ{};

	/// DELTA ILE DLUGOSCI PROMIENIA MIESCI SIE W JEDNYM BLOKU
	float DeltaX = abs(1.0f / CameraFront.x); // ODLEGLOSC O ILE BLOKOW MUSI PRZESUNAC SIE PROMIEN ABY PRZESUNAC SIE O JEDEN BLOK W OSI X 
	float DeltaY = abs(1.0f / CameraFront.y); // ODLEGLOSC O ILE BLOKOW MUSI PRZESUNAC SIE PROMIEN ABY PRZESUNAC SIE O JEDEN BLOK W OSI Y
	float DeltaZ = abs(1.0f / CameraFront.z); // ODLEGLOSC O ILE BLOKOW MUSI PRZESUNAC SIE PROMIEN ABY PRZESUNAC SIE O JEDEN BLOK W OSI Z

	float SideDistX{};
	float SideDistY{};
	float SideDistZ{};

	/// W ZALEZNOSCI GDZIE PATRZY SIE KAMERA ZMIENIAMY ZMIENNE

	if (CameraFront.x < 0)
	{
		StepX = -1;
		// TERAZ GDY PATRZYMY SIE W LEWO
		SideDistX = (CameraPosition.x - x) * DeltaX;
	}
	else
	{
		StepX = 1;
		// TERAZ GDY PATRZYMY SIE W PRAWO
		SideDistX = (x + 1.0f - CameraPosition.x) * DeltaX;
	}

	if (CameraFront.y < 0)
	{
		StepY = -1;
		// TERAZ GDY PATRZYMY SIE W DOL
		SideDistY = (CameraPosition.y - y) * DeltaY;
	}
	else
	{
		StepY = 1;
		// TERAZ GDY PATRZYMY DO GORY

		SideDistY = (y + 1.0f - CameraPosition.y) * DeltaY;
	}

	if (CameraFront.z < 0)
	{
		// TERAZ GDY PATRZYMY DO TYLU
		StepZ = -1;
		SideDistZ = (CameraPosition.z - z) * DeltaZ;
	}
	else
	{
		// TERAZ GDY PATRZYMY PROSTO
		StepZ = 1;
		SideDistZ = (z + 1.0f - CameraPosition.z) * DeltaZ;
	}

	float distance{};

	while (distance <= MaxDistance)
	{
		BlockHit = false;
		if (SideDistX < SideDistY && SideDistX < SideDistZ)
		{
			x += StepX;
			SideDistX += DeltaX;
			distance = SideDistX - DeltaX;
		}
		else if (SideDistY < SideDistX && SideDistY < SideDistZ)
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

		if (distance > MaxDistance)
			break;


		BlockID currentBlockID = world->getBlock(x, y, z);

		if (currentBlockID != 0) // CZY TRAFIONO BLOK 
		{
			auto now = std::chrono::steady_clock::now();

			if (now - lastHitTime >= hitCooldown)
			{
			
			std::cout << "GIGA ASIGMA";
			lastHitTime = now;
			}

			HitBlockPosition = { x,y,z };
			BlockHit = true;
			break;
		}


	}


}