#include "KeyTarget.h"
#include "KeyActivator.h"


KeyTarget::KeyTarget()
{
}

KeyTarget::KeyTarget(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget)
{
	if (tokens.size() > 4) { // Key is both a target and activator, so get info for target
		int targetID = std::stoi(tokens[4]);
		int threshold = std::stoi(tokens[5]);

		setThreshold(threshold);
		(*idToTarget)[targetID] = this;
	}
	else { // key is only activator so allow it to be picked up
		canBePickedUp = true;
	}
}

KeyTarget::~KeyTarget()
{
}

