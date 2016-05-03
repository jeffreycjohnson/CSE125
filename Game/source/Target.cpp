#include "Target.h"

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
	return ((positives - negatives) >= activationThreshold) && !isZeroed;
}
