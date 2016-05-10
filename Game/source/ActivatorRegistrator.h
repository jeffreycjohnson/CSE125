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

typedef std::function<Target*(std::vector<std::string>, std::map<int, Target*>*)> targFun;
typedef std::function<Activator*(std::vector<std::string>, const std::map<int, Target*>&)> actvFun;

class ActivatorRegistrator : public Component
{
public:
	static std::map<std::string, targFun> prefixToTarget;
	static std::map<std::string, actvFun> prefixToActivator;
	ActivatorRegistrator();
	~ActivatorRegistrator();

	void create() override;
};

#endif // ACTIVATOR_REGISTRATOR_H

