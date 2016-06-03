#pragma once
#include "UI.h"

class UIButton : public UIComponent {
private:
	int centerPosX, centerPosY;
	int spriteWidth, spriteHeight;

	std::string defaultStateName;
	Texture* defaultState;

	// Events
	std::function<void()> click;

public:

	UIButton(const std::string&, int x, int y, int w, int h);
	~UIButton();

	void loadTextures() override;

	// Event Listener modifiers
	// Usage:
	//    button.onClick() = std::bind(ClassName::foo, this);
	std::function<void()>& onClick();

	// All logic goes here
	void drawUI() override;
};