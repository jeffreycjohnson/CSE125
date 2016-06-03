#include "UI.h"

void UIList::add(const std::string & name, UIComponent* component)
{
	components[name] = component;
}

UIComponent* UIList::get(const std::string & name)
{
	return components[name];
}

void UIList::clear()
{
	components.clear();
}

void UIList::drawUI()
{
	for (auto pair : components) {
		auto uiComponent = pair.second;
		if (uiComponent != nullptr)
			uiComponent->drawUI();
	}
}
