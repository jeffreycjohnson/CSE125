#ifndef INCLUDE_SOUND_H
#define INCLUDE_SOUND_H

#include "ForwardDecs.h"
#include "Component.h"
#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"
#include "NetworkManager.h"
#include "NetworkUtility.h"
#include <unordered_map>
#include <queue>

typedef FMOD::Sound* SoundClass;

class Sound : public Component
{
private:
	std::string name;
	FMOD::Channel* channel;
	float volume;
	glm::vec3 position, prevPosition, velocity;
	bool looping, playing, active, is3D;
	int channelType;
	bool isConstructed;

	static FMOD_RESULT result; // For debugging
	static bool broadcasting;
	static std::queue<Sound*> broadcastQueue;

	// Do not allow use of this constructor outside of Sound
	// (you can crash the game this way)
	Sound() {
		isConstructed = false;
	};

public:
	// Channel group for sound effects
	static const int SOUND_EFFECT = 0;

	// Channel group for music
	static const int MUSIC = 1; 

	// Special channel group that lowers master gain (except for on itself) of everything else
	static const int BROADCAST = 2;

	static FMOD::ChannelGroup *cgEffects, *cgMusic, *cgBroadcast, *cgGame, *masterChannelGroup;
	static Sound* currentMusic;

	Sound(std::string soundName, bool playOnAwake, bool looping, float volume, bool is3D, int type = SOUND_EFFECT);
	void postConstructor();
	~Sound();

	void update(float);
	void play();
	void pause();
	void stop();
	void toggle();
	void setLooping(bool looping, int count);
	void setVolume(float volume);

	// This checks if the sound is actually being played on a channel in FMOD
	// It does NOT check the FMOD flag!
	bool isPlaying();

	//initFromConfig is broken, Do not touch, just use the macro!!!
	static void initFromConfig();

	// Useful for holding multiple sound components in one gameobject
	static Sound* affixSoundToDummy(GameObject*, Sound*);

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