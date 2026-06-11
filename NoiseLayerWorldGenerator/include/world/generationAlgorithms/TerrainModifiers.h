#pragma once
#include "world/TerrainPipeline.h"

class ModifierInvert : public TerrainModifier {
public:
	float modify(float value) override;
	void renderImGui() override;
};


class ModifierPower : public TerrainModifier {
public:
	float exponent = 2.0f;
	float modify(float value) override;
	void renderImGui() override;
};


class ModifierTerrace : public TerrainModifier {
public:
	int steps = 5;
	float modify(float value) override;
	void renderImGui() override;
};


class ModifierRidged : public TerrainModifier {
public:
	float modify(float value) override;
	void renderImGui() override;
};


class ModifierMesaCurve : public TerrainModifier {
public:
	float valleyFlatness = 0.3f;
	float plateauFlatness = 0.5f;

	float modify(float value) override;
	void renderImGui() override;
};

