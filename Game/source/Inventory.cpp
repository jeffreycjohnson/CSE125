#include "Inventory.h"
#include "GameObject.h"


Inventory::Inventory()
{
}


Inventory::~Inventory()
{
}

bool Inventory::hasKey()
{
	return key != nullptr;
}

GameObject * Inventory::getKey()
{
	return key;
}

void Inventory::setKey(GameObject * key)
{
	if (key != nullptr) {
		this->key = key;
		//key->transform.setPosition(); // set position to hand position
	}
}

void Inventory::removeKey() {
	if (this->key != nullptr) {
		this->key->destroy();
		this->key = nullptr;
	}
}
