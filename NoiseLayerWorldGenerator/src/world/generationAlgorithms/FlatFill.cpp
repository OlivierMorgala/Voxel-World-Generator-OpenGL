#include "world/generationAlgorithms/FlatFill.h"

float FlatFill::evaluate(float x, float z){
	return 1.0f;
}

float FlatFill::evaluate3D(float x, float y, float z) {
    return 1.0f;
}

void FlatFill::renderImGui() {
	ImGui::Text("NO SETTINGS");
}

void FlatFill::setSeed(int newSeed) {

}
