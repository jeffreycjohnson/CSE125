#include "Crosshair.h"
#include "Renderer.h"
#include <iostream>

Crosshair::Crosshair(const std::string& texture) : draw(true) {
	crosshairImage = std::make_unique<Texture>(texture, true);
}

bool Crosshair::drawUI(CrosshairNetworkData::CrosshairState state)
{
	if (draw == true) {
		auto center = glm::vec2(Renderer::getWindowWidth() / 2.0f, Renderer::getWindowHeight() / 2.0f);
		Renderer::drawSprite(center, glm::vec2(0.44, 0.25), glm::vec4(1), crosshairImage.get());
		std::cout << "Crosshair state changed to " << state << std::endl;
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

// Called by FPSMovement to set State based on what you are looking at
void Crosshair::setState(CrosshairNetworkData::CrosshairState state, int clientID)
{
	// Send network message to client to set crosshair state
	CrosshairNetworkData c = CrosshairNetworkData(state);
	std::vector<char> bytes = structToBytes(c);
	NetworkManager::PostMessage(bytes, CROSSHAIR_NETWORK_DATA, clientID, clientID);
}

CrosshairNetworkData::CrosshairState Crosshair::getState()
{
	return currentState;
}


void Crosshair::Dispatch(const std::vector<char> &bytes, int messageType, int messageId) {
	// decode struct
	CrosshairNetworkData cnd = structFromBytes<CrosshairNetworkData>(bytes);
	drawUI(cnd.state);
}

