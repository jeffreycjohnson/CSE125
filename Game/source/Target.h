#ifndef TARGET_H
#define TARGET_H

#include "Component.h"
#include "GameObject.h"

#include "Activator.h"
#include <string>
#include <map>

enum TriggerType;


class TargetFactory
{
public:
	virtual Target *create() = 0;
};


class Target :
	public Component
{
private:
	bool isZeroed;
	int activationThreshold;

	int positives;
	int negatives;
	static std::map<std::string, TargetFactory*> factories;

public:
	Target();
	Target(int activationThreshold);
	virtual ~Target() = 0;

	void applyTrigger(TriggerType triggerType);
	void releaseTrigger(TriggerType triggerType);

	bool isActivated();

	void setThreshold(int threshold) { activationThreshold = threshold; }

	static void registerType(
		const std::string& name, TargetFactory *factory)
	{
		factories[name] = factory;
	}

};

#endif // TARGET_H

