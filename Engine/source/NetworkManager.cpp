#include "NetworkManager.h"

#include <iostream>
#include <stdexcept>

#include "Component.h"
#include "GameObject.h"
#include "Mesh.h"
#include "Transform.h"

#include "ClientNetwork.h"
#include "ServerNetwork.h"
#include "ServerInput.h"

NetworkManager::NetworkManager()
{
	NetworkManager::state = UNINITIALIZED;
}

NetworkManager::~NetworkManager()
{
	NetworkManager::state = SHUTDOWN;
}

// --- GENERAL FUNC -- //

NetworkState NetworkManager::state;
std::vector<ClientID> NetworkManager::clientIDs;
ClientID NetworkManager::myClientID;

std::vector<NetworkResponse> NetworkManager::postbox;

// --- SERVER FUNC --- //

std::vector<ClientID> NetworkManager::InitializeServer(std::string port, int numberOfClients)
{
	assert(NetworkManager::state == UNINITIALIZED);
	NetworkManager::state = INITIALIZING;

	// setup the server for listening
	ServerNetwork::setup(port);

	// wait until all clients have registered
	NetworkManager::clientIDs = ServerNetwork::startMultiple(numberOfClients);

	// send out all clear
	for (auto clientID : clientIDs)
	{
		ClientsConnNetworkData cnd;
		cnd.connected = 1;
		cnd.numClients = clientIDs.size();
		cnd.yourClientID = clientID;

		std::vector<char> bytes = structToBytes(cnd);

		ServerNetwork::sendBytes(clientID, bytes, CLIENTS_CONN_NETWORK_DATA, clientID);
	}

	// and when we're all done
	GameObject::AddPreFixedUpdateCallback(NetworkManager::ReceiveServerMessages);
	GameObject::AddPostFixedUpdateCallback(NetworkManager::SendServerMessages);

	NetworkManager::state = SERVER_MODE;
	return NetworkManager::clientIDs;
}

void NetworkManager::ReceiveServerMessages()
{
	assert(NetworkManager::state == SERVER_MODE);

	// for now, all received messages are client-side input
	std::vector<std::vector<NetworkResponse>> multiResponses = ServerNetwork::selectClients();
	if (multiResponses.empty()) return;

	// loop through all of the clients' responses
	for (auto& clientResponses : multiResponses)
	{
		if (clientResponses.empty()) continue;

		// grab the last one only
		// BECAUSE WE'RE ONLY HANDLING INPUT!!
		NetworkResponse final = clientResponses.back();
		if (final.messageType == INPUT_NETWORK_DATA)
		{
			ServerInput::deserializeAndApply(final.body, final.id);
		}
	}
}

void NetworkManager::SendServerMessages()
{
	assert(NetworkManager::state == SERVER_MODE);

	// TODO: replace with like good polling, because this is just terrible
	for (ClientID clientID : NetworkManager::clientIDs)
	{
		std::string playerName = std::string("player_") + std::to_string(clientID);
		GameObject *correctPlayer = GameObject::FindByName(playerName);
		std::vector<char> bytes = correctPlayer->transform.serialize();
		int playerID = correctPlayer->getID();

		ServerNetwork::broadcastBytes(bytes, TRANSFORM_NETWORK_DATA, playerID);
	}
}

void NetworkManager::postMessage(std::vector<char> bytes, int messageType, int messageID)
{
	assert(NetworkManager::state != UNINITIALIZED &&
		NetworkManager::state != INITIALIZING &&
		NetworkManager::state != SHUTDOWN);

	if (NetworkManager::state != SERVER_MODE)
	{
		return;
	}
}

// --- CLIENT FUNC --- //

std::tuple<std::vector<ClientID>, ClientID> NetworkManager::InitializeClient(std::string serverIP, std::string port)
{
	assert(NetworkManager::state == UNINITIALIZED);
	NetworkManager::state = INITIALIZING;

	// setup the connection
	ClientNetwork::setup(serverIP, port);

	// WAIT FOR THE ALL CLEAR
	bool isConnected = false;
	while (!isConnected)
	{
		std::vector<NetworkResponse> messages = ClientNetwork::receiveMessages();

		for (auto& response : messages)
		{
			if (response.messageType == CLIENTS_CONN_NETWORK_DATA)
			{
				ClientsConnNetworkData *c = (ClientsConnNetworkData *)response.body.data();
				if (!c->connected) continue;

				NetworkManager::myClientID = c->yourClientID;
				for (int clientID = 0; clientID < c->numClients; clientID++)
				{
					NetworkManager::clientIDs.push_back(clientID);
				}

				std::cout << "All clients connected!" << std::endl;
				isConnected = true;
				break;
			}
			else if (response.body.size() != 0)
			{
				throw std::runtime_error("Wrong message received before all clear...");
			}
		}
	}

	// and when we're all done
	GameObject::AddPreFixedUpdateCallback(NetworkManager::ReceiveClientMessages);
	GameObject::AddPostFixedUpdateCallback(NetworkManager::SendClientMessages);

	NetworkManager::state = CLIENT_MODE;
	return std::make_tuple(NetworkManager::clientIDs, NetworkManager::myClientID);
}

void NetworkManager::ReceiveClientMessages()
{
	assert(NetworkManager::state == CLIENT_MODE);

	std::vector<NetworkResponse> receivedMessages = ClientNetwork::receiveMessages();
	for (auto& received : receivedMessages)
	{
		int& msgType = received.messageType;

		switch (msgType)
		{
		case CLIENTS_CONN_NETWORK_DATA:
			std::cerr << "RECEIVED ALL CLEAR MESSAGE AFTER SERVER INITIALIZATION" << std::endl;
			break;
		case TRANSFORM_NETWORK_DATA:
			Transform::Dispatch(received.body, received.messageType, received.id);
			break;
		case MESH_NETWORK_DATA:
			Mesh::Dispatch(received.body, received.messageType, received.id);
			break;
		case CREATE_OBJECT_NETWORK_DATA:
		case DESTROY_OBJECT_NETWORK_DATA:
			GameObject::Dispatch(received.body, received.messageType, received.id);
			break;
		default:
			std::cerr << "Client received message of type " << msgType << ", don't know what to do with it..." << std::endl;
		}
	}
}

void NetworkManager::SendClientMessages()
{
	assert(NetworkManager::state == CLIENT_MODE);

	std::vector<char> bytes = Input::serialize(NetworkManager::myClientID);
	ClientNetwork::sendBytes(bytes, INPUT_NETWORK_DATA, NetworkManager::myClientID);
}

// --- OFFLINE FUNC -- //

void NetworkManager::InitializeOffline()
{
	assert(NetworkManager::state == UNINITIALIZED);
	NetworkManager::state = OFFLINE;
}
