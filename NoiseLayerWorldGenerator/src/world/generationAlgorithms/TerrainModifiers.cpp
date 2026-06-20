#include "world/generationAlgorithms/TerrainModifiers.h"

//ModifierInvert---------------------- // Odwraca wysokosc terenu 
float ModifierInvert::modify(float value) {
	return 1.0f - value;
}

void ModifierInvert::renderImGui() {
	ImGui::Text("Invert");
}
//------------------------------------


//ModifierPower---------------------- // Modyfikuje wysokosc za pomoca potegowania, jesli 
// Czym wy¿szy wykladnik (exponent) tym zwieksza ogolna wysokosc -> wysokie gory i szczyty. Maly wykladnik tworzy bardzo plaski teren
 
float ModifierPower::modify(float value) {
	return std::pow(value ,exponent);
}

void ModifierPower::renderImGui() {
	ImGui::DragFloat("Power", &exponent, 0.05f, 0.1f, 10.0f);
}
//------------------------------------


//ModifierTerrace---------------------- // Modyfikuje teren -> tworzy schodkowy warstwowy teren podobny do tarasow
float ModifierTerrace::modify(float value) {
	return std::floor(value * steps) / static_cast<float>(steps);
}

void ModifierTerrace::renderImGui() {
	ImGui::DragInt("Steps count", &steps, 1, 1, 50);
}
//------------------------------------


//ModifierRidged---------------------- // Przeksztalca teren w strome ostre krawedzie i granie
float ModifierRidged::modify(float value) {
	float centered = value * 2.0f - 1.0f;
	return 1.0f - std::abs(centered);
}

void ModifierRidged::renderImGui() {
	ImGui::Text("Ridged");
}
//------------------------------------


//ModifierMesaCurve---------------------- // tworzy strome plaskowyze mesa o zaokraglonych zboczach
float ModifierMesaCurve::modify(float value) {
	if (value <= valleyFlatness) { return 0.0f; }

	if (value >= plateauFlatness) { return 1.0f; }

	// Zapobiega dzieleniu przez zero lub ujemnym przedzialom
	if (valleyFlatness >= plateauFlatness) { return value; }

	float t = (value - valleyFlatness) / (plateauFlatness - valleyFlatness);

	// Interpolacja smoothstep wielomian zapewnia gladkie przejscie i zaokraglenie (lagodny start i koniec) eliminuje nienaturalne  ostre krawedzie
	return (t * t * (3.0f - 2.0f * t));
}

void ModifierMesaCurve::renderImGui() {
	ImGui::DragFloat("Valleys width", &valleyFlatness, 0.01f, 0.0f, plateauFlatness - 0.01f);
	ImGui::DragFloat("Plateaus width", &plateauFlatness, 0.01f, valleyFlatness + 0.01f, 1.0f);
}
//------------------------------------


