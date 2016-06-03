#ifndef LASER_H
#define LASER_H

#include "Target.h"
#include "Sound.h"
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
	std::vector<Sound*> passiveHum;

	~Laser();

	void fixedUpdate() override;
	void collisionStay(GameObject * other) override;
	void create() override;
};

#endif // LASER_H
