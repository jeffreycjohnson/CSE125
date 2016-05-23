#include "PressButton.h"

#include "Timer.h"

PressButton::PressButton()
{
}

PressButton::PressButton(std::vector<std::string> tokens, const std::map<int, Target*>& idToTargets)
	: isActivated(isActivated), timeLeft(0.0f)
{
	timeLimit = std::stof(tokens[1]);

	for (int i = 2; i < tokens.size(); i += 3)
	{
		int targetID = std::stoi(tokens[i + 0]);
		TriggerType triggerType = strToTriggerType(tokens[i + 1]);
		int activatorID = std::stoi(tokens[i + 2]);

		this->addConnection(Connection(idToTargets.at(targetID), triggerType));
	}
}

PressButton::~PressButton()
{
}

void PressButton::fixedUpdate()
{
	float deltaTime = Timer::fixedTimestep;

	if (isActivated && timeLimit > 0)
	{
		if (timeLeft < 0)
		{
			isActivated = false;
			timeLeft = 0.0f;

			deactivate();
		}
	}
}

void PressButton::trigger()
{
	if (!isActivated)
	{
		isActivated = true;
		timeLeft = timeLimit == 0 ? 123456 : timeLimit;

		activate();
	}
}
