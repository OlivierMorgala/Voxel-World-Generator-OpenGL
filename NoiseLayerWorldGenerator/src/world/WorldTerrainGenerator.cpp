#include "world/WorldTerrainGenerator.h"
#include "world/Chunk.h"
#include "world/WorldConfig.h"

void WorldTerrainGenerator::applyToColumn(ChunkColumn& column) {
    //Przeliczamy współrzędne kolumny na współrzędne w świecie
    int worldX = column.getX() * Chunk::CHUNK_SIZE;
    int worldZ = column.getZ() * Chunk::CHUNK_SIZE;

    //Maksymalna wysokosc świata ograniczona do wysokości ustalonej na podstawie warst generacji (zapisana jest w konfigu)
    int maxWorldHeight = config.worldHeightInChunks * Chunk::CHUNK_SIZE - 1;

    //Przechodzimy po każdym słupie w kolumnie
    for (int x = 0; x < Chunk::CHUNK_SIZE; x++) {
        for (int z = 0; z < Chunk::CHUNK_SIZE; z++) {

            //Zmienna przchowująca wysokośc terenu ułożoną z poprzednich warstw 2D
            float currentHeight = 0.0f;

            //Nakładamy na słupki bloki generwoane przez każdą kolejną warstwe
            for (const TerrainLayer& layer : generationLayers) {
                if (!layer.isEnabled || !layer.algorithm) { continue; }

                //Jeśli tryb mieszania warstwy wymusza działanie w 3D rozpatrujemy go inaczej
                if (layer.blendMode == BlendMode::CARVE || layer.blendMode == BlendMode::CARVEIN) {

                    //Odcinamy wartość w przedziałach świata dla bezpieczeństwa
                    int absoluteBottom = std::clamp(layer.startY, 0, maxWorldHeight);
                    int absoluteTop = std::clamp(layer.endY, 0, maxWorldHeight);


                    for (int y = absoluteBottom; y <= absoluteTop; y++) {

                        //Tryb CARVEIN wycina tylko w blokach o konkretnym id
                        if (layer.blendMode == BlendMode::CARVEIN) {
                            if (column.getBlock(x, y, z) != layer.targetBlockID) {
                                continue;
                            }
                        }

                        //Pobranie wartości 3D dla danego szumu
                        float noise3DValue = layer.algorithm->evaluate3D(worldX + x, static_cast<float>(y), worldZ + z);

                        //Przetwarzamy otrzymana warotść przez modyfikatory warstwy
                        for (const auto& modification : layer.activeModifiers) {
                            noise3DValue = modification->modify(noise3DValue);
                        }

                        //Jeśli szum przewyższa próg wagi stawaiamy blok (tryb CARVE)
                        if (noise3DValue > layer.blendWeight) {
                            column.setBlock(x, y, z, layer.blockID);
                        }
                    }

                    //Koniec dizałania dla tej warstwy
                    continue;
                }

                //Obliczamy wartość szumu dala warstwy 2D
                float value = layer.algorithm->evaluate(worldX + x, worldZ + z);

                //Przetwarzamy otrzymana warotść przez modyfikatory warstwy
                for (const auto& modification : layer.activeModifiers) {
                    value = modification->modify(value);
                }

                //Mapujemy wynik z przedziału 0 do 1 nam konkretną wysokość warstwy
                float mappedHeight = layer.startY + (value * (layer.endY - layer.startY));

                float newHeight = currentHeight;

                //Mieszanie matamatyczne nakładających sie warst
                switch (layer.blendMode) {
                    case BlendMode::NORMAL:newHeight = mappedHeight; //Bezpośrednie nadpisanie (Używać zazwyczaj jedynie jako podsatwowa warstwa)
                        break;
                    case BlendMode::ADD:newHeight = currentHeight + mappedHeight; //Podniesieni bloków o X (usypwyanie dodatkowe terenu poagurki śnieg itp)
                        break;
                    case BlendMode::SUBTRACT:newHeight = currentHeight - mappedHeight; //Wycinanie bloków o X (wycinanie rzek lub kraterów)
                        break;
                    case BlendMode::MULTIPLY:newHeight = currentHeight * (mappedHeight / (float)maxWorldHeight); //Mnożenie obecnej wysokości przez wysokosć nowej warstwy (jako ułamek całkowitej wysokości świata)
                        break;
                    case BlendMode::MAX:newHeight = std::max(currentHeight, mappedHeight); //Zachowanie wyższej partii (do tworzenia ostrych gór przebijających sie przez inne warstwy)
                        break;
                    case BlendMode::MIN:newHeight = std::min(currentHeight, mappedHeight); //Zachowanie niższej partii (do towrzenia płaskich dolin albo ścinania gór)
                        break;
                    case BlendMode::SMOOTH:newHeight = currentHeight + layer.blendWeight * (mappedHeight - currentHeight); //Płynne przejście z starej wartości do nowej (wygłądzanie terenu)
                        break;
                    case BlendMode::ABSOLUTE:newHeight = std::max(currentHeight, mappedHeight);  //Warstwa absolutna która działa na określonym przedziale i zastępuje wszystko (podobne do maks głównie do eksperymentów albo chmur)
                        break;
                }

                // Zabezpieczenie przed alokacją poza tablicami w osi Y
                int iCurrent = std::clamp(static_cast<int>(currentHeight), 0, maxWorldHeight);
                int iNew = std::clamp(static_cast<int>(newHeight), 0, maxWorldHeight);

                //Ustawianie bloków dla trybu absolutnego (ustawiamy bloki na wyliczonych wysokościach)
                if (layer.blendMode == BlendMode::ABSOLUTE) {

                    //PO ZMIANY gdy dodana zostanie zmienna z maksymalną wysokością
                    int absoluteBottom = std::clamp(layer.startY, 0, 255);
                    int absoluteTop = std::clamp(static_cast<int>(mappedHeight), 0, 255);

                    //Stawaimy bloki w danym wyznaczonym przedziale
                    for (int y = absoluteBottom; y <= absoluteTop; y++) {
                        column.setBlock(x, y, z, layer.blockID);
                    }
                }
                else {
                    //Obliczenia dal pozostałych trybów wzgledem starej i nowej wysokości

                    if (iNew > iCurrent) {
                        //Jeśli teren rośnie to dodajemy nowe bloki na góre
                        for (int y = iCurrent; y < iNew; y++) {
                            column.setBlock(x, y, z, layer.blockID);
                        }
                    }
                    else if (iNew < iCurrent) {
                        //Jeśli teren jest ucinnany to usuwamy stare bloki i zastępujemy je powietrzem
                        for (int y = iNew; y < iCurrent; y++) {
                            column.setBlock(x, y, z, 0);
                        }
                    }
                }

                //Aktualizujemy górny poziom dla kolejnych algorytmów
                currentHeight = newHeight;
            }

        }
    }
}

void WorldTerrainGenerator::clearLayers() {
    generationLayers.clear();
}