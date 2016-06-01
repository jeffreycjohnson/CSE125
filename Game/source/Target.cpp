#include "Target.h"

Target::Target()
{
}

Target::Target(int activationThreshold)
	: activationThreshold(activationThreshold)
{
}

Target::~Target()
{
}

void Target::applyTrigger(TriggerType triggerType)
{
	switch (triggerType)
	{
	case POSITIVE:
		positives += 1;
		break;
	case NEGATIVE:
		negatives += 1;
		break;
	case ZERO:
		isZeroed = true;
		break;
	}
}

void Target::releaseTrigger(TriggerType triggerType)
{
	switch (triggerType)
	{
	case POSITIVE:
		positives -= 1;
		break;
	case NEGATIVE:
		negatives -= 1;
		break;
	case ZERO:
		isZeroed = false;
		break;
	}

	positives = std::max(positives, 0);
	negatives = std::max(negatives, 0);
}

bool Target::isActivated()
{
	return isHacked || ((activationLevel() >= activationThreshold) && !isZeroedOut());
}

int Target::activationLevel()
{
	return positives - negatives;
}

bool Target::isZeroedOut()
{
	return isZeroed;
}

void Target::hack()
{
	isHacked = true;
}

void Target::unhack()
{
	isHacked = false;
}
