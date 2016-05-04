#ifndef LASER_H
#define LASER_H

#include "Target.h"

class Laser :
	public Target
{
public:
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
	void collisionEnter(GameObject * other) override;
	void collisionStay(GameObject * other) override;

};

#endif // LASER_H
