#ifndef PRESS_BUTTON_H
#define PRESS_BUTTON_H

#include "Activator.h"


class PressButton :
	public Activator
{
private:
	const float buttonTime = 3.0f;

	bool isActivated;
	float timeLeft;
public:
	PressButton();
	PressButton(std::vector<std::string> tokens, const std::map<int, Target*>& idToTargets);

	~PressButton();

	void fixedUpdate() override;

	void trigger();
};

#endif 