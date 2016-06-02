#include "Crosshair.h"
#include "Renderer.h"
#include "Input.h"
#include <iostream>

static bool cursorOverride = false;

Crosshair::Crosshair(const std::string& defaultTexture, const std::string& interactiveTexture) : draw(true) {
	defaultCrosshairImage = std::make_unique<Texture>(defaultTexture, true);
	interactiveCrosshairImage = std::make_unique<Texture>(interactiveTexture, true);
	currentState = CrosshairNetworkData::CrosshairState::DEFAULT;
}

bool Crosshair::drawUI()
{
	if (Input::getKeyDown("`")) {
		cursorOverride = !cursorOverride;
	}

	if (draw == true && !cursorOverride) {
		auto center = glm::vec2(Renderer::getWindowWidth() / 2.0f, Renderer::getWindowHeight() / 2.0f);
		
		switch (currentState) {

		case CrosshairNetworkData::CrosshairState::DEFAULT:
			Renderer::drawSprite(center, glm::vec2(0.44, 0.25), glm::vec4(1), defaultCrosshairImage.get());
			break;
		case CrosshairNetworkData::CrosshairState::INTERACTIVE:
			Renderer::drawSprite(center, glm::vec2(0.44, 0.25), glm::vec4(1), interactiveCrosshairImage.get());
			break;
		}

	}
	return false;
}

void Crosshair::show()
{
	draw = true;
}

void Crosshair::hide()
{
	draw = false;
}

CrosshairNetworkData::CrosshairState Crosshair::getState()
{
	return currentState;
}

// Called by FPSMovement to set State based on what you are looking at
void Crosshair::setState(CrosshairNetworkData::CrosshairState state, int clientID)
{
	// Send network message to client to set crosshair state
	CrosshairNetworkData c = CrosshairNetworkData(state);
	std::vector<char> bytes = structToBytes(c);
	Renderer::crosshair->currentState = state;
	NetworkManager::PostMessage(bytes, CROSSHAIR_NETWORK_DATA, clientID, clientID);
}


void Crosshair::Dispatch(const std::vector<char> &bytes, int messageType, int messageId) {
	// decode struct
	CrosshairNetworkData cnd = structFromBytes<CrosshairNetworkData>(bytes);
	currentState = cnd.state;
}

