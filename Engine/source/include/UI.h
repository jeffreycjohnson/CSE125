#pragma once
#include "Component.h"
#include <vector>
#include <string>
#include <map>
#include <functional>

class UIComponent {
protected:
	bool texturesLoaded;
public:
	UIComponent() : texturesLoaded(false) {};

	virtual void loadTextures();
	virtual void drawUI();
};

class UIList {
private:
	std::map<std::string, UIComponent*> components;

public:
	void add(const std::string&, UIComponent*);
	UIComponent* get(const std::string&);

	void clear();
	void drawUI();
};

class UIMenu : public Component {
protected:
	UIList elements;

public:
	UIMenu();
	~UIMenu();

	virtual void create();
	virtual bool drawUI();
};