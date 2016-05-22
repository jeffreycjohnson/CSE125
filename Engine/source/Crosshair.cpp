#include "Crosshair.h"
#include "Renderer.h"

Crosshair::Crosshair(const std::string& texture) {
	crosshairImage = std::make_unique<Texture>(texture, true);
}

void Crosshair::drawUI()
{
	//Renderer::drawSprite(glm::vec2(-1, 1), glm::vec2(1, Renderer::getWindowWidth() / Renderer::getWindowHeight()) * 0.02f, glm::vec4(1), crosshairImage.get());
	auto scale = glm::vec2(1);
	Renderer::drawSprite(glm::vec2(0), scale, glm::vec4(1), crosshairImage.get());
	Renderer::drawSprite(glm::vec2(16), scale, glm::vec4(1), crosshairImage.get());
	Renderer::drawSprite(glm::vec2(128), scale, glm::vec4(1), crosshairImage.get());
	Renderer::drawSprite(glm::vec2(256), glm::vec2(scale), glm::vec4(1), crosshairImage.get());
	Renderer::drawSprite(glm::vec2(640, 480), glm::vec2(scale), glm::vec4(1), crosshairImage.get());
}
