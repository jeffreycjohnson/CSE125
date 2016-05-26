#pragma once
#include "Component.h"
#include "Texture.h"

class Crosshair {
private:
	std::unique_ptr<Texture> crosshairImage;
	bool draw;

public:
	Crosshair(const std::string& texture);

	bool drawUI();
	void show();
	void hide();
};