#include "Inventory.h"
#include "GameObject.h"
#include <iostream>
#include "KeyActivator.h"
#include "KeyTarget.h"
#include "FixedKeyTarget.h"

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
	if (key != nullptr && !hasKey()) {
		std::cout << "Picked up key for keyHole " << key->getComponent<KeyActivator>()->keyHoleID << std::endl;
		this->key = key;
		GameObject * hand = this->gameObject->transform.children[0]->children[1]->gameObject;
		hand->addChild(key);
		key->transform.setPosition(0.0f, 0.0f, 0.0f);
		FixedKeyTarget * fixedKeyTarget = key->getComponent<FixedKeyTarget>();
		KeyTarget * keyTarget = key->getComponent<KeyTarget>();
		if (fixedKeyTarget != nullptr) {
			fixedKeyTarget->pickedUp = true;
		}
		else if (keyTarget != nullptr) {
			keyTarget->pickedUp = true;
		}

	}
}

void Inventory::removeKey() {
	if (this->key != nullptr) {
		std::cout << "removing key from inventory" << std::endl;
		GameObject * hand = this->gameObject->transform.children[0]->children[1]->gameObject;
		hand->removeChild(key);
		((GameObject *)this->key)->destroy();
		this->key = nullptr;
	}
}

