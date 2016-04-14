#include "ClientManager.h"

#include <iostream>
#include <string>

#include "ClientNetwork.h"
#include "Input.h"

void ClientManager::sendMessages()
{
	std::string inputMessage = Input::serializeAsString();
	ClientNetwork::sendMessage(inputMessage);

	std::cout << "sent message " << inputMessage << std::endl;
}

void ClientManager::receiveMessages()
{
	std::string received = ClientNetwork::receiveMessage();
	std::cout << "recv message " << received << std::endl;
}