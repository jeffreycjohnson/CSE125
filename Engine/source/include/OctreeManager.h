#pragma once
#include "Component.h"
#include "Collision.h"

class OctreeManager :
	public Component
{
public:
	OctreeManager();
	~OctreeManager();

	void fixedUpdate() override;
};

