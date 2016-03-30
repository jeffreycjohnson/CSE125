#ifndef INCLUDE_ANIMATION_H
#define INCLUDE_ANIMATION_H

#include "ForwardDecs.h"
#include "Component.h"
#include <vector>
#include <unordered_map>

#include <gtc/quaternion.hpp>
#include <assimp/scene.h>           // Output data structure

struct KeyframeData {
	std::vector<std::pair<float, glm::vec3>> position;
	std::vector<std::pair<float, glm::quat>> rotation;
	std::vector<std::pair<float, glm::vec3>> scale;
};

struct AnimNodeData {
	std::string name;
	Transform* object;
	KeyframeData keyframes;
	float animationTime;
};

struct AnimationData {
	float animationTime;
	std::vector<AnimNodeData> boneData;
};

class Animation :
	public Component
{
public:
	
	Animation(const aiScene*, std::unordered_map<std::string, Transform*>);
	~Animation();

	void play(int animation, bool loop);
	void stop();

	const std::vector<AnimNodeData>& getAnimationData();

	void update(float dt);
private:
	std::vector<AnimationData> animData;
	int currentAnimationIndex = 0;
	float currentTime = 0;
	bool playing = false;
	bool looping = false;
};

#endif
