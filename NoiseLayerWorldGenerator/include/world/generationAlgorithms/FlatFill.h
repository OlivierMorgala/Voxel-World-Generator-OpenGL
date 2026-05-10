#pragma once

#include "GenerationAlgorithm.h"
class FlatFill : public GenerationAlgorithm
{
private:

public:
	FlatFill(std::string name, int startY, int endY);

	virtual void applyToColumn(ChunkColumn& column) override;
	virtual void renderImGuiSettings() override;
};

