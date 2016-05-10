#ifndef INVENTORY_H
#define INVENTORY_H

#include "Component.h"
class Inventory :
	public Component
{
private:
	GameObject * key;
public:
	Inventory();
	~Inventory();

	bool hasKey();
	GameObject * getKey();
	void setKey(GameObject * key);
	void removeKey();
};

#endif // INVENTORY_H
