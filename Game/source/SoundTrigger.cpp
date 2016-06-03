#include "SoundTrigger.h"
#include "SoundTrigger.h"

#include "Sound.h"

SoundTrigger::SoundTrigger(std::vector<std::string> tokens)
{
	soundName = tokens[1];

	for (int t = 2; t < tokens.size(); t++)
	{
		soundName += "_";
		soundName += tokens[t];
	}

	
}

SoundTrigger::~SoundTrigger()
{
}

void SoundTrigger::create()
{
	snd = Sound::affixSoundToDummy(this->gameObject, new Sound(soundName, false, false, 1.0f, false, Sound::BROADCAST));
}

void SoundTrigger::collisionEnter(GameObject * other)
{
	if (!playedOnce)
	{
		playedOnce = true;
		snd->play();
	}
}
