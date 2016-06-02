#pragma once

#include "Activator.h"
#include "Collision.h"
#include "Sound.h"

class SoundTrigger :
	public Activator
{
private:
	std::string soundName;
	Sound *snd;
	bool playedOnce = false;

public:
	SoundTrigger(std::vector<std::string> tokens);
	~SoundTrigger();

	void create() override;

	void collisionEnter(GameObject * other) override;
};

