#ifndef INCLUDE_SOUND_H
#define INCLUDE_SOUND_H

#include "ForwardDecs.h"
#include "Component.h"
#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"
#include <unordered_map>
#include "NetworkManager.h"
#include "NetworkUtility.h"

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
		isConstructed = false;
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

	//initFromConfig is broken, Do not touch, just use the macro!!!
	static void initFromConfig();

	static FMOD::System *system;
	static std::unordered_map<std::string, FMOD::Sound*> soundMap;
	static void init();
	static void updateFMOD();
	static void FMODErrorCheck(FMOD_RESULT result, const std::string& msg);
	static void Dispatch(const std::vector<char>& bytes, int messageType, int messageId);

	void deserializeAndApply(std::vector<char> bytes) override;
	void setGameObject(GameObject* object) override;

	std::vector<char> serialize(SoundNetworkData::soundState ss, bool loopingParam, int count, float volumeParam);
protected:
	void postToNetwork(SoundNetworkData::soundState ss, bool loopingParam, int count, float volumeParam);
};

#endif