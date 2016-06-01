#include "PressButton.h"
#include "Timer.h"
#include "Config.h"

PressButton::PressButton()
{
}

PressButton::PressButton(std::vector<std::string> tokens, const std::map<std::string, Target*>& idToTargets, std::string groupName)
	: isActivated(isActivated), timeLeft(0.0f)
{
	timeLimit = std::stof(tokens[1]);
	
	// get around the fact that blender names things xxxx.001, xxxx.002 etc
	int size = 0;
	if (tokens[tokens.size() - 1].size() == 3) {
		size = tokens.size() - 1;
	}
	else {
		size = tokens.size();
	}

	for (int i = 2; i < size; i += 3)
	{
		int targetID = std::stoi(tokens[i + 0]);
		TriggerType triggerType = strToTriggerType(tokens[i + 1]);
		int activatorID = std::stoi(tokens[i + 2]);

		this->addConnection(Connection(idToTargets.at(groupName + std::to_string(targetID)), triggerType));
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
		timeLeft -= deltaTime;
		snd_activate->play();

		if (timeLeft < 0)
		{
			isActivated = false;
			timeLeft = 0.0f;

			deactivate();
			snd_deactivate->play();
		}
	}
}

void PressButton::create()
{
	GameObject* go;
	if ((go = gameObject->findChildByNameContains("Colliders")) == nullptr) {
		go = gameObject;
	}
	ConfigFile file("config/sounds.ini");
	snd_activate = Sound::affixSoundToDummy(go, new Sound("button_activate", false, false, file.getFloat("button_activate", "volume"), true));
	snd_deactivate = Sound::affixSoundToDummy(go, new Sound("button_deactivate", false, false, file.getFloat("button_deactivate", "volume"), true));
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
