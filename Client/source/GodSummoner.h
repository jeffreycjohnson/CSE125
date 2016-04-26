#ifndef GOD_SUMMONER_H
#define GOD_SUMMONER_H

#include "Component.h"

class GodSummoner : public Component
{
private:
	bool rotating;
	GameObject *toRotate;
public:
	GodSummoner(GameObject *toRotate);
	~GodSummoner();

	void update(float deltaTime) override;
	void collisionEnter(GameObject *other) override;
	void collisionStay(GameObject *other) override;
	void collisionExit(GameObject *other) override;

	void setTarget(GameObject *target);
};

#endif

