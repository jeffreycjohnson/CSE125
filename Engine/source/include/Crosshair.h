#pragma once
#include "Component.h"
#include "Texture.h"

class Crosshair : public Component {
private:
	std::unique_ptr<Texture> crosshairImage;
public:
	Crosshair(const std::string& texture);

	bool drawUI() override;
};