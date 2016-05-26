#include "SoundListener.h"

#include "Sound.h"

SoundListener::SoundListener()
{
}


SoundListener::~SoundListener()
{
}


void SoundListener::fixedUpdate()
{
	Sound::setListenerPosition(this->gameObject);
}