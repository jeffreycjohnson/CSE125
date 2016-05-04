#ifndef LASER_H
#define LASER_H

#include "Target.h"
#include <vector>

class Laser :
	public Target
{
public:
	Laser();

	Laser(std::vector<std::string> tokens);
	Laser(int activationThreshold);
	~Laser();

	void fixedUpdate() override;
};

class FixedLaser :
	public Target
{
private:
	bool areLasersOff;
public:
	FixedLaser(std::vector<std::string> tokens);

	FixedLaser() {}
	FixedLaser(int activationThreshold);
	~FixedLaser();

	void fixedUpdate() override;
};

#endif // LASER_H
