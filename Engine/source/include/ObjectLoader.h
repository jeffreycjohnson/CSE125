#pragma once
#include "ForwardDecs.h"
#include <unordered_map>
#include <functional>

// returns the root game object of the file, with all children correctly added
GameObject* loadScene(const std::string& filename);

extern std::unordered_multimap<std::string, std::function<void(GameObject*)>> componentMap;

template<typename T, typename U>
void addComponentMapping(T name, U func)
{
    componentMap.insert(std::make_pair(name, func));
}