#ifndef ACTIVATOR_REGISTRATOR_H
#define ACTIVATOR_REGISTRATOR_H

#include "Component.h"
#include "Target.h"
#include "Activator.h"
#include <map>
#include <vector>
#include <unordered_map>
#include <functional>
#include <string>

class ActivatorRegistrator : public Component
{
public:
	static std::map<std::string, std::function<Target*(std::vector<std::string>)>> prefixToTarget;
	static std::map<std::string, std::function<Activator*(std::vector<std::string>)>> prefixToActivator;
	ActivatorRegistrator();
	~ActivatorRegistrator();

	void create() override;
};

#endif // ACTIVATOR_REGISTRATOR_H

