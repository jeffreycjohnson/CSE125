#include "Sound.h"
#include "GameObject.h"
#include "fmod/fmod.hpp"
#include "fmod/fmod_dsp.h"
#include "fmod/fmod_errors.h"
#include "Input.h"
#include "Config.h"
#include "Renderer.h"
#include "Camera.h"
#include "Timer.h"

#include <iostream>

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

	this->postConstructor();
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
		velocity = (position - prevPosition) * Timer::fixedTimestep;

		FMOD_VECTOR pos = { position.x, position.y, position.z };
		FMOD_VECTOR vel = { velocity.x, velocity.y, velocity.z };

		if (is3D) {
			channel->set3DAttributes(&pos, &vel, 0);
		}
		prevPosition = gameObject->transform.getWorldPosition();
	}
	channel->setPaused(!playing); // Used at the end of update to prevent inconsistent initial volume
}

void Sound::play()
{
	//std::cout << "SOUND: " << name << std::endl;
	if (playing)
	{
		// Possible leak, does FMOD handle deleting sound instances for playSound?
		result = system->playSound(soundMap[name], 0, true, &channel);

		position = gameObject->transform.getWorldPosition(); // Needs to be in world position for audio
		velocity = (position - prevPosition) * Timer::fixedTimestep;
		FMOD_VECTOR pos = { position.x, position.y, position.z };
		FMOD_VECTOR vel = { velocity.x, velocity.y, velocity.z };
		
		if (is3D) {
			channel->set3DAttributes(&pos, &vel, 0);
		}
		prevPosition = gameObject->transform.getWorldPosition();

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

		if (!is3D) {
			channel->setPriority(0);
		}
		channel->setPaused(!playing); // play the sound
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
	if (channel != nullptr) {
		FMODErrorCheck(channel->setVolume(volume), "FMOD set volume failed.");
	}
	postToNetwork(SoundNetworkData::soundState::SET_VOLUME, false, -1, volume);
}



void Sound::init()
{
	FMODErrorCheck(FMOD::System_Create(&system), "FMOD::System_Create failed");
	
	int driverCount = 0;
	result = system->getNumDrivers(&driverCount);

	if (driverCount == 0 || result != FMOD_OK)
	{
		FATAL("FMOD failed to find sound drivers (or worse.)");
	}

	// Initialize our Instance with 128 channels
	FMODErrorCheck( system->init(256, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, NULL), "FMOD system->init() failed.");
	FMODErrorCheck( system->set3DNumListeners(1), "Failed to set # of 3D listeners");

	// Generate sound map
	initFromConfig();

}

void Sound::updateFMOD()
{
	// Listener information is now updated in here as opposed in Camera
	// Apparently this is more consistent. Hopefully.

	glm::vec3 position = Renderer::mainCamera->gameObject->transform.getWorldPosition();
	
	// Velocity is 0 for now. If doppler effect desired, please calculate in terms of meters per second
	glm::vec3 velocity = { 0.0, 0.0, 0.0 };
	glm::vec3 forward  = glm::normalize(glm::vec3(Renderer::mainCamera->gameObject->transform.getTransformMatrix() * glm::vec4(0, 0, -1, 0)));
	glm::vec3 up = glm::normalize(glm::cross({-1, 0, 0}, forward));

	FMOD_VECTOR pos = { position.x, position.y, position.z };
	FMOD_VECTOR vel = { velocity.x, velocity.y, velocity.z };
	FMOD_VECTOR fwd = { forward.x, forward.y, forward.z };
	FMOD_VECTOR upv = { up.x, up.y, up.z };

	FMODErrorCheck(system->set3DListenerAttributes(0, &pos, &vel, &fwd, &upv), "FMOD Error while updating 3D listener attrs.");

	FMODErrorCheck(system->update(), "FMOD failure in system->update()");
}

void Sound::FMODErrorCheck(FMOD_RESULT result, const std::string& msg)
{
	if (result != FMOD_OK) {
		FATAL(msg.c_str());
	}
}

void Sound::Dispatch(const std::vector<char> &bytes, int messageType, int messageId) {
	GameObject *go = GameObject::FindByID(messageId);
	if (go == nullptr)
	{
		throw std::runtime_error("From Sound.cpp/Dispatch: Nonexistant gameobject");
	}
	Sound * s;
	s = go->getComponent<Sound>();
	if (s != nullptr) {
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
		FATAL("I technically don't need a mutate state.");
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

Sound* Sound::affixSoundToDummy(GameObject* parent, Sound * sound)
{
	auto dummy = new GameObject;
	dummy->setName(sound->name);
	dummy->addComponent(sound);
	parent->addChild(dummy);
	return sound;
}

bool Sound::isPlaying()
{
	// Detect if the sound is still playing
	FMOD::Sound* ptr;
	if (channel != nullptr) {
		channel->getCurrentSound(&ptr);
		if (ptr == soundMap[name]) {
			bool paused;
			channel->getPaused(&paused);
			return paused;
		}
		else {
			return false;
		}
	}
	else
		return false;
}

void Sound::initFromConfig()
{
	ConfigFile file("config/sounds.ini");
	std::string list = file.getString("SoundList", "soundlist");
	std::vector<std::string> sounds = file.allSections();

	for (auto i = sounds.begin(); i != sounds.end(); ++i) {
		SoundClass soundToAdd;
		std::string fileName = file.getString(*i, "file");
		std::string fmodMode = file.getString(*i, "fmodMode");

		int is2DElse3D = fmodMode == std::string("2D") ? FMOD_2D : FMOD_3D;
		//int exinfo = file.getInt(*i, "exinfo");
		//TODO: Don't know what exinfo is
		//TODO: TEST THIS!!! DON'T KNOW IF YOUR REFERENCES WILL DISAPPEAR
		FMODErrorCheck( system->createSound(fileName.c_str(), is2DElse3D, NULL, &soundToAdd), fileName );
		soundMap.insert({ *i, soundToAdd });
	}
	//Sanity check
	std::cout << "Sound's initFromConfig Sanity Check" << std::endl;
	for (auto i : soundMap) {
		std::cout << i.first << std::endl;
		std::cout << i.second << std::endl;
	}
	std::cout << std::endl << std::endl;
}