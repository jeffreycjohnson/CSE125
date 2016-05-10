#ifndef ROTATING_H
#define ROTATING_H

#include "Target.h"

class Rotating :
	public Target
{
public:
	Rotating();
	Rotating(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget);

	Rotating(int activationThreshold);
	~Rotating();

	void fixedUpdate() override;
};

#endif // ROTATING_H

