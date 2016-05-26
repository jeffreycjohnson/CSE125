#include "Crosshair.h"
#include "Renderer.h"

Crosshair::Crosshair(const std::string& texture) {
	crosshairImage = std::make_unique<Texture>(texture, true);
}

bool Crosshair::drawUI()
{
	Renderer::drawSprite(glm::vec2(-1, 1), glm::vec2(1, Renderer::getWindowWidth() / Renderer::getWindowHeight()) * 0.02f, glm::vec4(1), crosshairImage.get());
	return false;
}
