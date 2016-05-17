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
	Laser(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget);

	~Laser();

	void fixedUpdate() override;
	void collisionStay(GameObject * other) override;
};

class Forcefield : public Target
{
public:
	Forcefield(std::vector<std::string> tokens, std::map<int, Target*>* idToTarget);
	void fixedUpdate() override;
};

#endif // LASER_H
