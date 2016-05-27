#pragma once
#include "Component.h"
#include "Texture.h"
#include "NetworkStruct.h"

class Crosshair {

private:
	std::unique_ptr<Texture> defaultCrosshairImage;
	std::unique_ptr<Texture> interactiveCrosshairImage;
	bool draw;
	CrosshairNetworkData::CrosshairState currentState;

public:
	Crosshair(const std::string& texture, const std::string& interactiveTexture);

	bool drawUI();
	void show();
	void hide();

	static void setState(CrosshairNetworkData::CrosshairState state, int clientID);
	void Dispatch(const std::vector<char> &bytes, int messageType, int messageId);

};