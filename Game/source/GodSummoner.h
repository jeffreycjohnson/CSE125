#ifndef GOD_SUMMONER_H
#define GOD_SUMMONER_H

#include "Target.h"

class GodSummoner : public Target
{
private:
public:
	GodSummoner(std::vector<std::string> tokens, std::map<std::string, Target*>* idToTarget, std::string groupName);
	~GodSummoner();

	void create() override;
	void fixedUpdate() override;
};

#endif

