#include "PressButton.h"
#include "Timer.h"
#include "Config.h"
#include "Light.h"

PressButton::PressButton()
{
}

PressButton::PressButton(std::vector<std::string> tokens, const std::map<std::string, Target*>& idToTargets, std::string groupName)
	: isActivated(isActivated), timeLeft(0.0f)
{
	timeLimit = ((float) std::stoi(tokens[1])) / 1000.0f;
	
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
		snd_tick->play();

		if (timeLeft < 0)
		{
			isActivated = false;
			timeLeft = 0.0f;

			deactivate();
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
	snd_activate = Sound::affixSoundToDummy(go, new Sound("button_activate", false, false, file.getFloat("button_activate", "volume"), true, Sound::NO_DAMPEN_CHANNEL));
	snd_tick = Sound::affixSoundToDummy(go, new Sound("button_tick", false, false, file.getFloat("button_tick", "volume"), true, Sound::NO_DAMPEN_CHANNEL));
}

void PressButton::trigger()
{
	if (!isActivated)
	{
		isActivated = true;
		timeLeft = timeLimit == 0 ? 123456 : timeLimit;

		activate();

		// Hack to make prison lights flash when prison button is pressed
		if (timeLimit == 0) {
			snd_activate->play();

			std::vector<GameObject *> goVec = GameObject::FindAllByPrefix("PrisonLight");
			for (auto go : goVec) {
				FlashingLightsNetworkData flnd = FlashingLightsNetworkData(glm::vec3(0.5, 0, 0), go->getID());
				NetworkManager::PostMessage(structToBytes(flnd), FLASHING_LIGHTS_NETWORK_DATA, 0);
			}
		}
	}
}
