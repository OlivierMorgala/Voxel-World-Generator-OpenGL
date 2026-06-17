#include "world/WorldRenderer.h"

WorldRenderer::WorldRenderer()
{
	//Prealokujemy przewidywaną wartosć kolumn w wektorze pomaga zapobiec lagom na początku i zapobiega rozszerzaniu sie o 1.5 na początku 
	visibleColumns.reserve(3000);
}

void WorldRenderer::render(World& world, const Camera& camera, Shader& shader, float windowAspectRatio)
{

	shader.useShader();

	//Przeakzujemy macierze widoku i perspektywy do shadera
	shader.setMatrix4("view", camera.getViewMatrix());
	shader.setMatrix4("projection", camera.getProjectionMatrix(windowAspectRatio));

	//Przeliczamy pozaycje gracza na indeksy kolumn (indeksy w których znajduje sie kamera)
	int cameraColumnX = static_cast<int>(std::floor(camera.position.x / (Chunk::CHUNK_SIZE)));
	int cameraColumnZ = static_cast<int>(std::floor(camera.position.z / (Chunk::CHUNK_SIZE)));

	//flaga sprawdzająca czy kamera jest w bloku przeźroczystym
	isCameraUnderwater = false;
	underwaterColor = glm::vec3(0.0f);

	//Obliczamy w jakim dokłądnie bloku jest kamera (koordynaty)
	int cameraBlockX = static_cast<int>(std::floor(camera.position.x));
	int cameraBlockY = static_cast<int>(std::floor(camera.position.y));
	int cameraBlockZ = static_cast<int>(std::floor(camera.position.z));

	//Sprawdzamy czy bloki w kótym jest kamera to blok przeźroczysty i zmieniamy flage
	std::shared_ptr<ChunkColumn> cameraColumn = world.getChunkColumn(cameraColumnX, cameraColumnZ);
	if (cameraColumn) {
		int localX = cameraBlockX % Chunk::CHUNK_SIZE;
		if(localX < 0) { localX += Chunk::CHUNK_SIZE; }
		int localZ = cameraBlockZ % Chunk::CHUNK_SIZE;
		if (localZ < 0) { localZ += Chunk::CHUNK_SIZE; }

		BlockID currentBlockID = cameraColumn->getBlock(localX, cameraBlockY, localZ);

		if(currentBlockID != 0 && BlockDatabase::getBlockData(currentBlockID).isTransparent){
			isCameraUnderwater = true;
			underwaterColor = BlockDatabase::getBlockData(currentBlockID).color;
		}
	}

	//Renderowanie nieprzeźroczystych bloków
	shader.setValue("alpha", 1.0f); //ustawiamy alfe na 1 (brak przeźroczystości)
	shader.setValue("isBorderRendered", true); //usatwaiamy uniform na renderowanie obramówek bloków
	shader.setValue("showColumnBorders", config.showChunkColumnsBorder); //???
	shader.setValue("chunkSize", static_cast<float>(Chunk::CHUNK_SIZE)); //przesyłamy rozamir chunka do shadera

	//Kofiguracaj stanu renderu bloków 
	glEnable(GL_CULL_FACE); //nie rysujemy tylnich scian bloków (te wewnątrz)
	glDepthMask(GL_TRUE);  //zezwalamy na zapis głebokości (obeikty z przodyu przysłąniają te z tyłu)
	glDisable(GL_BLEND); //wyłączamy przeźroczystość

	visibleColumns.clear();

	//Przygotowujemy frustum dla naszej kamery
	Frustum frustumCamera;
	if (config.isFrustumCullingEnabled) {
		frustumCamera = camera.getFrustum(windowAspectRatio);
	}

	std::vector<ChunkColumn*> loadedColumns = world.getLoadedColumns();

	//Renderwoanie wybranych  kolumn
	for (ChunkColumn* column : loadedColumns) {

		//Obliczamy odległość danej kolumny od kamery w danej osi
		int dx = column->getX() - cameraColumnX;
		int dz = column->getZ() - cameraColumnZ;

		//Sprawdzamy czy kolumna znajduje się w promieniu Render Distance gracza
		if (std::abs(dx) <= config.renderDistance && std::abs(dz) <= config.renderDistance) {

			//Sprawdzamy czy kolumna ma siatke
			if (column->hasMesh()) {
				bool isVisible = true;

				//Filtrujemy kolmne przez nasz algorytm frustum
				if (config.isFrustumCullingEnabled) {
					AA_BoundingBox AABBcolumn;

					//Ustawiamy nasze wiszchołki kolumny na AABB (potrzeba tylko 2 by znać wszystkie)

					//Dolni lewy tylny róg
					AABBcolumn.min.x = column->getX() * Chunk::CHUNK_SIZE;
					AABBcolumn.min.y = 0.0f;
					AABBcolumn.min.z = column->getZ() * Chunk::CHUNK_SIZE;

					//Górny prawy przedni
					AABBcolumn.max.x = AABBcolumn.min.x + Chunk::CHUNK_SIZE;
					AABBcolumn.max.y = config.worldHeightInChunks * Chunk::CHUNK_SIZE;
					AABBcolumn.max.z = AABBcolumn.min.z + Chunk::CHUNK_SIZE;

					//Sprawdzamy czy chunk jest w polu widzenia kamery
					isVisible = camera.isAABoundingBoxVisible(frustumCamera, AABBcolumn);

				}

				//Jeśli chunk jest widoczny to go rysujemy
				if (isVisible) {
					column->renderOpaque(&shader);
					visibleColumns.push_back(column);
				}
			}
		}
	}

	shader.setValue("alpha", 0.65f); //ustawiamy lekką przeźroczystość
	shader.setValue("isBorderRendered", false); //wyłączamy rendeorwanie ramek

	glEnable(GL_BLEND); //włączamy mieszanie koloró (kanału alfa)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //usatwiamy typ mieszania
	glDepthMask(GL_FALSE); //wyłaczamy zapis do bufora głębokosci (dzieki temu woda bliżej kamery nie odcina bloków za nią)
	glDisable(GL_CULL_FACE); //wyłaczamy usuwanie tylnich ścian żaby było widać tafle wody patrząc od dołu

	//Renderujemy tylko te kolumny które przeszły test frustum
	for (ChunkColumn* column : visibleColumns) {
		column->renderTransparent(&shader);
	}

	//Resetujemy stany OpenGL do domyślnych
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
}

//Metoda zwarcająca widoczne kolumny
const std::vector<ChunkColumn*>& WorldRenderer::getVisibleColumns() const 
{
	return visibleColumns;
}