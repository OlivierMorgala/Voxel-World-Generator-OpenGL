#include "Raycasting.h"

Raycast::Raycast(float maxDistance, World* World, Shader* Shader) : maxDistance(maxDistance), world(World), shader(Shader), lastHitTime(std::chrono::steady_clock::now()) {}
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
	float DeltaX = (CameraFront.x ==  0) ? 1e30f : std::abs(1.0f / CameraFront.x);   // ODLEGLOSC O ILE BLOKOW MUSI PRZESUNAC SIE PROMIEN ABY PRZESUNAC SIE O JEDEN BLOK W OSI X 
	float DeltaY = (CameraFront.y == 0) ? 1e30f : std::abs(1.0f / CameraFront.y);// ODLEGLOSC O ILE BLOKOW MUSI PRZESUNAC SIE PROMIEN ABY PRZESUNAC SIE O JEDEN BLOK W OSI Y
	float DeltaZ = (CameraFront.z == 0) ? 1e30f : std::abs(1.0f / CameraFront.z); // ODLEGLOSC O ILE BLOKOW MUSI PRZESUNAC SIE PROMIEN ABY PRZESUNAC SIE O JEDEN BLOK W OSI Z

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
	BlockHit = false;
	
	shader->setValue("BlockHasHit", 0);
	while (distance <= MaxDistance)
	{
		
		if (SideDistX <= SideDistY && SideDistX <= SideDistZ)
		{
			x += StepX;
			SideDistX += DeltaX;
			distance += DeltaX;
		}
		else if (SideDistY <= SideDistX && SideDistY <= SideDistZ)
		{
			y += StepY;
			SideDistY += DeltaY;
			distance += DeltaY;
		}
		else
		{
			z += StepZ;
			SideDistZ += DeltaZ;
			distance += DeltaZ;
		}

		if (distance > MaxDistance)
			break;


		BlockID currentBlockID = world->getBlock(x, y, z);

		if (currentBlockID != 0) // CZY TRAFIONO BLOK 
		{
			// JESLI TRAFIONO BLOK TO ODCZEKUJEMY CHWILE A POTEM AKTUALIZUJEMY INFORMACJE O POZYCJI TRAFIONEGO BLOKU ORAZ PRZESY£AMY SHADEROWI INFORMACJE
			auto now = std::chrono::steady_clock::now();

			if (now - lastHitTime >= hitCooldown)
			{
			lastHitTime = now;
			}

			HitBlockPosition = { x,y,z };
			shader->setValueVec3("blockHitPosition", HitBlockPosition);
			shader->setValue("BlockHasHit", 1.0f);
			BlockHit = true;
			break;

			
		}


	}


}