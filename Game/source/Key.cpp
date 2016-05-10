#include "Key.h"



Key::Key()
{
}

Key::Key(std::vector<std::string> tokens, const std::map<int, Target*>& idToTargets)
{
}

Key::~Key()
{
}

void Key::trigger()
{
	// Key has been brought to KeyHole so activate KeyHoleTarget
	activate();
}
