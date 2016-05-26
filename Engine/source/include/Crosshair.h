#pragma once
#include "Component.h"
#include "Texture.h"
#include "NetworkStruct.h"

class Crosshair {

private:
	std::unique_ptr<Texture> crosshairImage;
	bool draw;
	CrosshairNetworkData::CrosshairState currentState = CrosshairNetworkData::CrosshairState::DEFAULT;

public:
	Crosshair(const std::string& texture);

	bool drawUI(CrosshairNetworkData::CrosshairState state);
	void show();
	void hide();

	static void setState(CrosshairNetworkData::CrosshairState state, int clientID);
	CrosshairNetworkData::CrosshairState getState();
	void Dispatch(const std::vector<char> &bytes, int messageType, int messageId);

};