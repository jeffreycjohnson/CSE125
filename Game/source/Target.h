#ifndef TARGET_H
#define TARGET_H

#include "Component.h"
#include "GameObject.h"

#include "Activator.h"

enum TriggerType;

class Target :
	public Component
{
private:
	bool isZeroed;
	int activationThreshold;

	int positives;
	int negatives;
public:
	Target(int activationThreshold);
	virtual ~Target() = 0;

	void applyTrigger(TriggerType triggerType);
	void releaseTrigger(TriggerType triggerType);

	bool isActivated();
};

#endif // TARGET_H

