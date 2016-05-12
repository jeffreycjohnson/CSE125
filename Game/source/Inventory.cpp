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

		// TODO: set position to hand position.
		// Currently just arbitrarily moving the key to visually indicate it has been picked up
		glm::vec3 playerPos = this->gameObject->transform.getPosition();
		glm::vec3 playerOffset = playerPos + glm::vec3(2.0f, 2.0f, 0.0f);
		key->transform.setPosition(playerOffset);
	}
}

void Inventory::removeKey() {
	if (this->key != nullptr) {
		std::cout << "removing key from inventory" << std::endl;
		((GameObject *)this->key)->destroy();
		this->key = nullptr;
	}
}

void Inventory::fixedUpdate()
{
	if (hasKey()) {
		// TODO: Adjust Keys position to match camera based on FPSMovement
	}
}
