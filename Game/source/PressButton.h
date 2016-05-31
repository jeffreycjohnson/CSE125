#ifndef PRESS_BUTTON_H
#define PRESS_BUTTON_H

#include "Activator.h"
#include "Sound.h"

class PressButton :
	public Activator
{
private:
	const float buttonTime = 3.0f;
	Sound *snd_activate, *snd_deactivate;
	bool isActivated;
	float timeLeft;
	float timeLimit;

public:
	PressButton();
	PressButton(std::vector<std::string> tokens, const std::map<std::string, Target*>& idToTargets, std::string groupName);

	~PressButton();

	void fixedUpdate() override;
	void create() override;

	void trigger();
};

#endif 