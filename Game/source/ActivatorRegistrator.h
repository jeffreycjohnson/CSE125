#ifndef ACTIVATOR_REGISTRATOR_H
#define ACTIVATOR_REGISTRATOR_H

#include "Component.h"

class ActivatorRegistrator : public Component
{
public:
	ActivatorRegistrator();
	~ActivatorRegistrator();

	void create() override;
};

#endif // ACTIVATOR_REGISTRATOR_H

