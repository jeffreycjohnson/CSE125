#include "UIButton.h"
#include "Texture.h"
#include "Input.h"
#include "Renderer.h"

#include <iostream>

UIButton::UIButton(const std::string & textureName, int x, int y, int w, int h) : centerPosX(x), centerPosY(y), spriteWidth(w), spriteHeight(h)
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

	int wHalf = spriteWidth / 2;
	int hHalf = spriteHeight / 2;
	int minX = centerPosX - wHalf;
	int maxX = centerPosX + wHalf;
	int minY = centerPosY - hHalf;
	int maxY = centerPosY + hHalf;

	// If mouse cursor is in the button's space
	int cursor_x = Input::mousePosition().x;
	int cursor_y = Input::mousePosition().y;

	//std::cerr << "mouse x,y: " << cursor_x << ", " << cursor_y << std::endl;

	if (cursor_x >= minX && cursor_x <= maxX &&
		cursor_y >= minY && cursor_y <= maxY && active) {
		if (Input::getMouseDown("mouse 0") && click != nullptr) {
			click();
			// TODO: draw click state instead of hover state or default, if we ever do that
		}
		Renderer::drawSprite(glm::vec2(centerPosX, centerPosY), glm::vec2(1), glm::vec4(1), defaultState); // TODO: draw hover state instead if we ever do that
	}
	else if (active) {
		Renderer::drawSprite(glm::vec2(centerPosX, centerPosY), glm::vec2(1), glm::vec4(1), defaultState);
	}

}
