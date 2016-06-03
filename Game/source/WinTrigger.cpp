#include "WinTrigger.h"
#include "Config.h"
#include "Renderer.h"

WinTrigger::WinTrigger(std::vector<std::string> tokens, const std::map<std::string, Target*>& idToTargets, std::string groupName)
{
	numClients = 4;
}

WinTrigger::~WinTrigger()
{
}

void WinTrigger::create()
{
	ConfigFile file("config/options.ini");
	numClients = file.getInt("NetworkOptions", "numclients");
	for (int i = 0; i < numClients; ++i) {
		this->playersEscaped.push_back(false);
	}
}

void WinTrigger::collisionEnter(GameObject * other)
{
	if (other != nullptr) {
		if (other->transform.getParent() != nullptr) {
			std::string name = other->transform.getParent()->gameObject->getName();
			auto underscore = name.find("_");
			if (underscore != std::string::npos) {
				std::string suffix = name.substr(underscore + 1);
				int suffixInt = std::atoi(suffix.c_str());
				playersEscaped[suffixInt] = true;
			}
		}
	}
}

void WinTrigger::fixedUpdate() {
	for (int i = 0; i < numClients; ++i) {
		if (!playersEscaped[i])
			return;
	}
	if (!everyoneEscaped) {
		NetworkManager::PostMessage(std::vector<char>(), GAME_END_EVENT, 0);
		everyoneEscaped = true;
	}
}