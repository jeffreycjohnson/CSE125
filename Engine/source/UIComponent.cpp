#include "UI.h"

void UIComponent::loadTextures()
{
}

void UIComponent::drawUI() {
	if (!texturesLoaded) {
		loadTextures();
		texturesLoaded = true;
	}
}