#include "UIButton.h"
#include "Texture.h"
#include "Input.h"
#include "Renderer.h"

UIButton::UIButton(const std::string & textureName, int x, int y) : centerPosX(x), centerPosY(y)
{
	defaultStateName = textureName;
	defaultState = nullptr;
}

UIButton::~UIButton() {
	if (defaultState != nullptr) {
		delete defaultState;
	}
}

void UIButton::loadTextures()
{
	// We need to load textures on the main thread
	defaultState = new Texture(defaultStateName, true);
	texturesLoaded = true;
}

std::function<void()>& UIButton::onClick()
{
	return click;
}

void UIButton::drawUI()
{
	if (!texturesLoaded)
		loadTextures();

	int wHalf = defaultState->getWidth() / 2;
	int hHalf = defaultState->getHeight() / 2;
	int minX = centerPosX - wHalf;
	int maxX = centerPosX + wHalf;
	int minY = centerPosY - hHalf;
	int maxY = centerPosY + hHalf;

	// If mouse cursor is in the button's space
	int cursor_x = Input::mousePosition().x;
	int cursor_y = Input::mousePosition().y;

	if (cursor_x >= minX && cursor_x <= maxX &&
		cursor_y >= minY && cursor_y <= maxY) {
		if (Input::getButtonDown("mouse 1") && click) {
			click();
			// TODO: draw click state instead of hover state or default, if we ever do that
		}
		Renderer::drawSprite(glm::vec2(centerPosX, centerPosY), glm::vec2(1), glm::vec4(1), defaultState); // TODO: draw hover state instead if we ever do that
	}
	else {
		Renderer::drawSprite(glm::vec2(centerPosX, centerPosY), glm::vec2(1), glm::vec4(1), defaultState);
	}

}
