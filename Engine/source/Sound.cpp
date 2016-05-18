#include "Sound.h"
#include "GameObject.h"
#include "fmod/fmod.hpp"
#include "fmod/fmod_dsp.h"
#include "fmod/fmod_errors.h"
#include <iostream>
#include "Input.h"
#include "Config.h"



FMOD::System* Sound::system;
std::unordered_map<std::string, FMOD::Sound*> Sound::soundMap;
FMOD_RESULT Sound::result;

Sound::Sound(std::string soundName, bool playOnAwake, bool looping, float volume, bool is3D)
{
	isConstructed = true;

	name = soundName;
	this->volume = volume;
	this->looping = looping;
	this->is3D = is3D;
	playing = active = playOnAwake;

	result = system->playSound(soundMap[name], 0, true, &channel);
	channel->setVolume(volume);
	if (!is3D)
		channel->setPriority(0);
	if (looping)
	{
		channel->setMode(FMOD_LOOP_NORMAL);
		channel->setLoopCount(-1);
	}
	else
	{
		channel->setMode(FMOD_LOOP_OFF);
	}
}

void Sound::postConstructor() {
	result = system->playSound(soundMap[name], 0, true, &channel);
	channel->setVolume(volume);
	if (!is3D)
		channel->setPriority(0);
	if (looping)
	{
		channel->setMode(FMOD_LOOP_NORMAL);
		channel->setLoopCount(-1);
	}
	else
	{
		channel->setMode(FMOD_LOOP_OFF);
	}
}

Sound::~Sound()
{
	
}

void Sound::update(float)
{
    if (playing) {
        position = gameObject->transform.getWorldPosition(); // Needs to be in world position for audio
        velocity = position - prevPosition;
        FMOD_VECTOR pos = { position.x, position.y, position.z };
        FMOD_VECTOR vel = { velocity.x, velocity.y, velocity.z };
        channel->set3DAttributes(&pos, &vel, 0);

        prevPosition = gameObject->transform.getWorldPosition();
    }

	channel->setPaused(!playing); // Used at the end of update to prevent inconsistent initial volume
}

void Sound::play()
{
	if (playing)
	{
		// Possible leak, does FMOD handle deleting sound instances for playSound?
		result = system->playSound(soundMap[name], 0, false, &channel);
		channel->setVolume(volume);
		if (looping)
		{
			channel->setMode(FMOD_LOOP_NORMAL);
			channel->setLoopCount(-1);
		}
		else
		{
			channel->setMode(FMOD_LOOP_OFF);
		}

		if (!is3D)
			channel->setPriority(0);
	}
	else // Paused
	{
		playing = true;
	}
	active = true;
	postToNetwork(SoundNetworkData::soundState::PLAY, false, -1, 0.0f);
}

void Sound::pause()
{
	playing = false;
	postToNetwork(SoundNetworkData::soundState::PAUSE, false, -1, 0.0f);
}

void Sound::stop()
{
	channel->stop();
	active = false;
	postToNetwork(SoundNetworkData::soundState::STOP, false, -1, 0.0f);
}

void Sound::toggle()
{
	if (active)
		stop();
	else
		play();
	postToNetwork(SoundNetworkData::soundState::TOGGLE, false, -1, 0.0f);
}

void Sound::setLooping(bool looping, int count = -1)
{
	if (looping)
		channel->setMode(FMOD_LOOP_NORMAL);
	else
		channel->setMode(FMOD_LOOP_OFF);

	channel->setLoopCount(count);
	postToNetwork(SoundNetworkData::soundState::SET_LOOPING, looping, count, 0.0f);
}

void Sound::setVolume(float volume)
{
	channel->setVolume(volume);
	postToNetwork(SoundNetworkData::soundState::SET_VOLUME, false, -1, volume);
}

void Sound::initFromConfig()
{
	ConfigFile file("config/sounds.ini");
	std::string list = file.getString("SoundList", "soundlist");
	std::vector<std::string> sounds;

	//Split
	size_t pos = 0;
	std::string token;
	while ((pos = list.find(";")) != std::string::npos) {
		token = list.substr(0, pos);
		sounds.push_back(token);
		list.erase(0, pos + 1);
	}

	for (auto i = sounds.begin(); i != sounds.end(); ++i) {
		SoundClass soundToAdd;
		std::string fileName = file.getString(*i, "file");
		std::string fmodMode = file.getString(*i, "fmodMode");
		int is2DElse3D = fmodMode == std::string("2D") ? FMOD_2D : FMOD_3D;
		//int exinfo = file.getInt(*i, "exinfo");
		//TODO: Don't know what exinfo is
		//TODO: TEST THIS!!! DON'T KNOW IF YOUR REFERENCES WILL DISAPPEAR
		system->createSound(fileName.c_str(), is2DElse3D, NULL, &soundToAdd);
		soundMap.insert({*i, soundToAdd});
	}
}

void Sound::init()
{
	result = FMOD::System_Create(&system);
	if (result != FMOD_OK)
	{
		std::cout << "FMOD Error " << result << std::endl;
		throw;
	}

	int driverCount = 0;
	result = system->getNumDrivers(&driverCount);

	if (driverCount == 0 || result != FMOD_OK)
	{
		std::cout << "FMOD Error " << result << std::endl;
		throw;
	}

	// Initialize our Instance with 128 channels
	system->init(256, FMOD_INIT_NORMAL, NULL);

#ifdef _SOUND_HARDCODE
	// Generate sound map
	SoundClass cabin;
	system->createSound("assets/sounds/ambience/cabin.wav", FMOD_2D, NULL, &cabin);
	soundMap.insert({ "cabin", cabin });
	SoundClass gun;
	system->createSound("assets/sounds/gun.wav", FMOD_2D, NULL, &gun);
	soundMap.insert({ "gun", gun });
	SoundClass boost;
	system->createSound("assets/sounds/boost.wav", FMOD_2D, NULL, &boost);
	soundMap.insert({ "boost", boost });
	SoundClass fighterEngine;
	system->createSound("assets/sounds/engine.wav", FMOD_3D, NULL, &fighterEngine);
	soundMap.insert({ "fighterEngine", fighterEngine });
	SoundClass explosion;
	system->createSound("assets/sounds/explosion.mp3", FMOD_3D, NULL, &explosion);
	soundMap.insert({ "explosion", explosion });
	SoundClass capital;
	system->createSound("assets/sounds/capital.wav", FMOD_2D, NULL, &capital);
	soundMap.insert({ "capital", capital });
	SoundClass music;
	system->createSound("assets/sounds/music/soundtrack.mp3", FMOD_2D, NULL, &music);
	soundMap.insert({ "music", music });
	// Add more sounds as we need
#else
	initFromConfig();
#endif
}

void Sound::updateFMOD()
{
	system->update();
}

void Sound::Dispatch(const std::vector<char> &bytes, int messageType, int messageId) {
	GameObject *go = GameObject::FindByID(messageId);
	if (go == nullptr)
	{
		throw std::runtime_error("From Sound.cpp/Dispatch: Nonexistant gameobject");
	}
	Sound * s;
	s = go->getComponent<Sound>();
	if (s == nullptr) {
		s->deserializeAndApply(bytes);
	}
	else {
		s = new Sound();
		go->addComponent(s);
		s->deserializeAndApply(bytes);
	}
}

void Sound::deserializeAndApply(std::vector<char> bytes){
	SoundNetworkData sind = structFromBytes<SoundNetworkData>(bytes);

	switch (sind.ss){
	case SoundNetworkData::soundState::PLAY:
		this->play();
		break;
	case SoundNetworkData::soundState::TOGGLE:
		this->toggle();
		break;
	case SoundNetworkData::soundState::STOP:
		this->stop();
		break;
	case SoundNetworkData::soundState::PAUSE:
		this->pause();
		break;
	case SoundNetworkData::soundState::SET_LOOPING:
		this->setLooping(sind.loopingParam, sind.count);
		break;
	case SoundNetworkData::soundState::SET_VOLUME:
		this->setVolume(sind.volumeParam);
		break;
	case SoundNetworkData::soundState::MUTATE:
		std::runtime_error("I technically don't need a mutate state.");
		break;
	case SoundNetworkData::soundState::CONSTRUCT:
	default:
		assert(isConstructed == false);
		name = std::string(sind.soundName);
		this->volume = sind.volume;
		this->looping = sind.looping;
		this->is3D = sind.is3D;
		this->playing = sind.playing;

		//This basically initializes things based off the 
		//old constructor
		this->postConstructor();
		this->isConstructed = true;
		break;
	}
}

std::vector<char> Sound::serialize(SoundNetworkData::soundState ss, bool loopingParam, int count, float volumeParam)
{
	SoundNetworkData snd = SoundNetworkData(
		gameObject->getID(),
		this->name,
		this->playing,
		this->active,
		this->looping,
		this->volume,
		this->is3D,
		ss,
		loopingParam,
		count,
		volumeParam
	);
	return structToBytes(snd);
}

void Sound::postToNetwork(SoundNetworkData::soundState ss, bool loopingParam, int count, float volumeParam)
{
	if (NetworkManager::getState() != SERVER_MODE) return;

	GameObject *my = gameObject;
	if (my == nullptr)
	{
		std::cerr << "Sound ain't got no attached game object modified??" << std::endl;
		return;
	}
	NetworkManager::PostMessage(serialize(ss, loopingParam, count, volumeParam), SOUND_NETWORK_DATA, my->getID());
}

void Sound::setGameObject(GameObject* object) {
	Component::setGameObject(object);
	postToNetwork(SoundNetworkData::soundState::CONSTRUCT, false, -1, 0.0f);
};
