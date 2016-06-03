#include "UI.h"

UIMenu::UIMenu()
{
}

UIMenu::~UIMenu()
{
	elements.clear();
}

void UIMenu::create()
{
	// Create your UI Elements, add them to the UIList
	// Ex:
	//     elements["Play"] = UIButton(...);

	// Here is where you would add event listeners for your elements:
	// Ex:
	//     elements["Play"].onClick(UIMenu::play, this);
}

bool UIMenu::drawUI()
{
	elements.drawUI();
	return false;
}