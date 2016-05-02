#ifndef ROTATING_H
#define ROTATING_H

#include "Target.h"

class Rotating :
	public Target
{
public:
	Rotating(int activationThreshold);
	~Rotating();

	void fixedUpdate() override;
};

#endif // ROTATING_H

