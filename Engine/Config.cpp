#include "Config.h"


Config::Config(std::string& configPath)
{
	if (instance == nullptr) {
		instance = this;
		configFilePath = configPath;
	}
}


Config::~Config()
{
}
