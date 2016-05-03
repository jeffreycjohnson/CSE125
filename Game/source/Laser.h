#ifndef LASER_H
#define LASER_H

#include "Target.h"

class Laser :
	public Target
{
public:
	Laser();
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
	FixedLaser(int activationThreshold);
	~FixedLaser();

	void fixedUpdate() override;
};

#endif // LASER_H
