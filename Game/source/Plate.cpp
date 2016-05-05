#include "Plate.h"

#include <iostream>

Plate::Plate()
{
	isNotColliding = true;
	isColliding = false;
}

Plate::Plate(std::vector<std::string> tokens)
{
	//TODO parse(tokens);
}

Plate::~Plate()
{
}

void Plate::fixedUpdate()
{
	// we're on the edge!!
	if (!isColliding && !isNotColliding)
	{
		isNotColliding = true;
		deactivate();
	}

	// we're on the edge!!
	if (isColliding && isNotColliding)
	{
		isNotColliding = false;
		activate();
		std::cout << "activated plate" << std::endl;
	}

	isColliding = false;
}

void Plate::collisionEnter(GameObject *other)
{
}

void Plate::collisionStay(GameObject *other)
{
	isColliding = true;
}