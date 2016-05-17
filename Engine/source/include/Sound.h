#ifndef INCLUDE_SOUND_H
#define INCLUDE_SOUND_H

#include "ForwardDecs.h"
#include "Component.h"
#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"
#include <unordered_map>

typedef FMOD::Sound* SoundClass;

class Sound : public Component
{
private:
	std::string name;
	FMOD::Channel* channel;
	float volume;
	glm::vec3 position, prevPosition, velocity;
	bool looping, playing, active, is3D;

	bool isConstructed;

	static FMOD_RESULT result; // For debugging

public:
	Sound() {
		isConstructed = true;
	};
	Sound(std::string soundName, bool playOnAwake, bool looping, float volume, bool is3D);
	void postConstructor();
	~Sound();

	void update(float);
	void play();
	void pause();
	void stop();
	void toggle();
	void setLooping(bool looping, int count);
	void setVolume(float volume);
	static void initFromConfig();

	static FMOD::System *system;
	static std::unordered_map<std::string, FMOD::Sound*> soundMap;
	static void init();
	static void updateFMOD();
	static void Dispatch(const std::vector<char>& bytes, int messageType, int messageId);
	void deserializeAndApply(std::vector<char> bytes) override;
	void sendEvent(int sstate, bool looping, int count, float volume);
};

#endif