#pragma once
#include "Activator.h"
#include "FPSMovement.h"

class WinTrigger : public Activator {
private:
	std::vector<bool> playersEscaped;
	int numClients;
	bool everyoneEscaped;

public:
	WinTrigger(std::vector<std::string> tokens, const std::map<std::string, Target*>& idToTargets, std::string groupName);
	~WinTrigger();

	void create() override;
	void fixedUpdate() override;
	void collisionEnter(GameObject * other) override;

};