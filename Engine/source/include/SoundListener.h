#pragma once
#include "Component.h"

class SoundListener :
	public Component
{
public:
	SoundListener();
	~SoundListener();

	void fixedUpdate() override;
};

