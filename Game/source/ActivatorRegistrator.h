#ifndef ACTIVATOR_REGISTRATOR_H
#define ACTIVATOR_REGISTRATOR_H

#include "Component.h"
#include <map>
#include <unordered_map>
#include <string>

class ActivatorRegistrator : public Component
{
public:
	static std::map<std::string, TargetFactory *> prefixToTarget;
	static std::map<std::string, ActivatorFactory *> prefixToActivator;
	ActivatorRegistrator();
	~ActivatorRegistrator();

	void create() override;
};

#endif // ACTIVATOR_REGISTRATOR_H

