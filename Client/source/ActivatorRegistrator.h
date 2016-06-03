#ifndef ACTIVATOR_REGISTRATOR_H
#define ACTIVATOR_REGISTRATOR_H

#include <set>
#include "Component.h"

class ActivatorRegistrator
{
private:
	void registerAll();

public:
	ActivatorRegistrator();
	~ActivatorRegistrator();
};

#endif // ACTIVATOR_REGISTRATOR_H

