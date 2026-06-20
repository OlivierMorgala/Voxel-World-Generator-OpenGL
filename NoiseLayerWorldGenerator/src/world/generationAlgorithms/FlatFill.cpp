#include "world/generationAlgorithms/FlatFill.h"

// FlatFill odpowiada za generowanie totalnie plaskiego podloza w swiecie

// Metoda evaluate: Metoda zwraca zawsze 1.0f 
float FlatFill::evaluate(float x, float z){
	return 1.0f;
}

float FlatFill::evaluate3D(float x, float y, float z) {
    return 1.0f;
}

// Metoda renderImGui:
void FlatFill::renderImGui() {
	ImGui::Text("NO SETTINGS");
}

void FlatFill::setSeed(int newSeed) {

}
