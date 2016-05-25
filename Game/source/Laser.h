#ifndef LASER_H
#define LASER_H

#include "Target.h"
#include <vector>

class Laser :
	public Target
{
private:
	bool isFixed;
	bool areLasersOff;

public:
	Laser();
	Laser(std::vector<std::string> tokens, std::map<std::string, Target*>* idToTarget, bool isFixed, std::string groupName);

	~Laser();

	void fixedUpdate() override;
	void collisionStay(GameObject * other) override;
};

#endif // LASER_H
