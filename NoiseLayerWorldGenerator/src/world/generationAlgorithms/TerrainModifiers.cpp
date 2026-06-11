#include "world/generationAlgorithms/TerrainModifiers.h"

//ModifierInvert----------------------
float ModifierInvert::modify(float value) {
	return 1.0f - value;
}

void ModifierInvert::renderImGui() {
	ImGui::Text("Invert");
}
//------------------------------------


//ModifierPower----------------------
float ModifierPower::modify(float value) {
	return std::pow(value ,exponent);
}

void ModifierPower::renderImGui() {
	ImGui::DragFloat("Power", &exponent, 0.05f, 0.1f, 10.0f);
}
//------------------------------------


//ModifierTerrace----------------------
float ModifierTerrace::modify(float value) {
	return std::floor(value * steps) / static_cast<float>(steps);
}

void ModifierTerrace::renderImGui() {
	ImGui::DragInt("Steps count", &steps, 1, 1, 50);
}
//------------------------------------


//ModifierRidged----------------------
float ModifierRidged::modify(float value) {
	float centered = value * 2.0f - 1.0f;
	return 1.0f - std::abs(centered);
}

void ModifierRidged::renderImGui() {
	ImGui::Text("Ridged Abs");
}
//------------------------------------


//ModifierMesaCurve----------------------
float ModifierMesaCurve::modify(float value) {
	float centered = value * 2.0f - 1.0f;
	return 1.0f - std::abs(centered);
}

void ModifierMesaCurve::renderImGui() {
	ImGui::DragFloat("Valleys width", &valleyFlatness, 0.01f, 0.0f, plateauFlatness - 0.01f);
	ImGui::DragFloat("Plateaus width", &plateauFlatness, 0.01f, valleyFlatness + 0.01f, 1.0f);
}
//------------------------------------


