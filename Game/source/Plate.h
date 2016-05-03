#ifndef PLATE_H
#define PLATE_H

#include "Activator.h"

class Plate :
	public Activator
{
private:
	// this is just necessary because no collisionExit
	bool isColliding;
	bool isNotColliding;
public:
	Plate();
	~Plate();

	void fixedUpdate() override;
	void collisionEnter(GameObject *other) override;
	void collisionStay(GameObject *other) override;
};

#endif //PLATE_H