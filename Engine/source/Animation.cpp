#include "Animation.h"

#include "Transform.h"


glm::vec3 interpolateKeyframes(std::vector<std::pair<float, glm::vec3>> data, float time) {
	int positionIndex = 0;
	int numPositions = data.size();
	glm::vec3 currentPosition;

	for (int index = numPositions - 1; index >= 0; --index) {
		if (data[index].first < time) {
			positionIndex = index;
			break;
		}
	}
	if (positionIndex + 1 < numPositions) {
		float t = (time - data[positionIndex].first) / (data[positionIndex + 1].first - data[positionIndex].first);
		currentPosition = data[positionIndex].second * (1 - t) + (t)*data[positionIndex + 1].second;
	}
	else {
		currentPosition = data[positionIndex].second;
	}
	return currentPosition;
}

glm::quat interpolateQuaternions(std::vector<std::pair<float, glm::quat>> data, float time) {
	int rotationIndex = 0;
	int numRotations = data.size();
	glm::quat currentRotation;

	for (int index = numRotations - 1; index >= 0; --index) {
		if (data[index].first < time) {
			rotationIndex = index;
			break;
		}
	}
	if (rotationIndex + 1 < numRotations) {
		float t = (time - data[rotationIndex].first) / (data[rotationIndex + 1].first - data[rotationIndex].first);
		currentRotation = glm::slerp(data[rotationIndex].second, data[rotationIndex + 1].second, t);
	}
	else {
		currentRotation = data[rotationIndex].second;
	}
	return currentRotation;
}

void Animation::play(int animation, bool loop) {
	currentAnimationIndex = animation;
	currentTime = 0;
	playing = true;
	looping = loop;
}

void Animation::stop() {
	playing = false;
}

const std::vector<AnimNodeData>& Animation::getAnimationData() {
	return animData[currentAnimationIndex].boneData;
}

void Animation::update(float dt) {
	AnimationData & currentAnim = animData[currentAnimationIndex];
	if (playing) {
		currentTime += dt;
		if (currentTime > currentAnim.animationTime) {
			if (looping) {
				currentTime -= currentAnim.animationTime;
			}
			else {
				stop();
			}
		}
	}
	for (AnimNodeData &node : currentAnim.boneData) {
		node.object->setPosition(interpolateKeyframes(node.keyframes.position, currentTime));
		node.object->setRotate(interpolateQuaternions(node.keyframes.rotation, currentTime));
		//node.object->scaleFactor = node.keyframes.scale[scaleIndex].second;
	}
}





glm::quat convertQuat(aiQuaternion input) {
	return glm::quat(input.w, input.x, input.y, input.z);
}

glm::vec3 convertVec(aiVector3D input) {
	return glm::vec3(input.x, input.y, input.z);
}

Animation::Animation(const aiScene* scene, std::unordered_map<std::string, Transform*> loadingAcceleration)
{
	for (unsigned int a = 0; a < scene->mNumAnimations; ++a) { //separate animations (e.g. run, jump)
        auto animation = scene->mAnimations[a];
		AnimationData currentAnimData;

		float longestTime = 0;
		//channels correspond to nodes, i.e. bones
		for (unsigned int c = 0; c < animation->mNumChannels; ++c) {
            auto channel = animation->mChannels[c];

			std::string name = channel->mNodeName.C_Str();

			AnimNodeData newData;
			newData.name = name;
			newData.object = loadingAcceleration[name];

			for (unsigned int keyframe = 0; keyframe < channel->mNumPositionKeys; ++keyframe) {
				newData.keyframes.position.push_back(std::make_pair(channel->mPositionKeys[keyframe].mTime,
					convertVec(channel->mPositionKeys[keyframe].mValue)));
			}

			for (unsigned int keyframe = 0; keyframe < channel->mNumRotationKeys; ++keyframe) {
				newData.keyframes.rotation.push_back(std::make_pair(channel->mRotationKeys[keyframe].mTime,
					convertQuat(channel->mRotationKeys[keyframe].mValue)));
			}

			for (unsigned int keyframe = 0; keyframe < channel->mNumScalingKeys; ++keyframe) {
				newData.keyframes.scale.push_back(std::make_pair(channel->mScalingKeys[keyframe].mTime,
					convertVec(channel->mScalingKeys[keyframe].mValue)));
			}

			if (newData.keyframes.position.back().first > longestTime) longestTime = newData.keyframes.position.back().first;
			if (newData.keyframes.rotation.back().first > longestTime) longestTime = newData.keyframes.rotation.back().first;
			if (newData.keyframes.scale.back().first > longestTime) longestTime = newData.keyframes.scale.back().first;

			currentAnimData.boneData.push_back(newData);
			
		}
		currentAnimData.animationTime = longestTime;
		animData.push_back(currentAnimData);
	}
}


Animation::~Animation()
{
}
