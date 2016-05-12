#include "Inventory.h"
#include "GameObject.h"
#include <iostream>
#include "Key.h"

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
		std::cout << "Picked up key for keyHole " << key->getComponent<Key>()->keyHoleID << std::endl;
		this->key = key;
		GameObject * hand = this->gameObject->transform.children[0]->children[1]->gameObject;
		hand->addChild(key);
		key->transform.setPosition(0.0f, 0.0f, 0.0f);
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

