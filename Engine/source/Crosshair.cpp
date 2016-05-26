#include "Crosshair.h"
#include "Renderer.h"

Crosshair::Crosshair(const std::string& texture) : draw(true) {
	crosshairImage = std::make_unique<Texture>(texture, true);
}

bool Crosshair::drawUI()
{
	if (draw == true) {
		auto center = glm::vec2(Renderer::getWindowWidth() / 2.0f, Renderer::getWindowHeight() / 2.0f);
		Renderer::drawSprite(center, glm::vec2(0.44, 0.25), glm::vec4(1), crosshairImage.get());
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
