#include "NetworkManager.h"

#include <iostream>
#include <stdexcept>

#include "Component.h"
#include "GameObject.h"
#include "Mesh.h"
#include "Camera.h"
#include "Transform.h"
#include "Light.h"

#include "NetworkStats.h"
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
std::vector<char> NetworkManager::lastBytesSent;

NetworkState NetworkManager::getState()
{
	return NetworkManager::state;
}

void NetworkManager::setState(NetworkState newState)
{
	state = newState;
}

// --- SERVER FUNC --- //

std::vector<ClientID> NetworkManager::InitializeServer(std::string port, int numberOfClients)
{
	if (NetworkManager::state != UNINITIALIZED)
	{
		throw std::runtime_error("Cannot initialize initialized networking.");
	}

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
	if (NetworkManager::state != SERVER_MODE)
	{
		throw std::runtime_error("Cannot receive messages on unitialized network");
	}

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

static int numServerMessages = 0;
void NetworkManager::SendServerMessages()
{
	if (NetworkManager::state != SERVER_MODE)
	{
		throw std::runtime_error("Cannot send messages on unitialized network");
	}

	std::map<int, std::vector<NetworkResponse>> perClient;

	// FOR OBJECT MANAGEMENT
	for (auto& response : NetworkManager::postbox)
	{
		if (response.messageType != CREATE_OBJECT_NETWORK_DATA &&
			response.messageType != DESTROY_OBJECT_NETWORK_DATA)
		{
			continue;
		}

		int& msgType = response.messageType;

		switch (msgType)
		{
		case CLIENTS_CONN_NETWORK_DATA:
			std::cerr << numServerMessages++ << "SEND ALL CLEAR MESSAGE AFTER SERVER INITIALIZATION" << std::endl;
			break;
		case TRANSFORM_NETWORK_DATA:
			// std::cerr << numClientMessages++ << "SEND TRANSFORM DATA" << std::endl;
			break;
		case MESH_NETWORK_DATA:
			std::cerr << numServerMessages++ << "SEND MESH DATA" << std::endl;
			break;
		case CAMERA_NETWORK_DATA:
			std::cerr << numServerMessages++ << "SEND CAMERA DATA" << std::endl;
			break;
		case LIGHT_NETWORK_DATA:
			std::cerr << numServerMessages++ << "SEND LIGHT DATA" << std::endl;
			break;
		case CREATE_OBJECT_NETWORK_DATA:
		case DESTROY_OBJECT_NETWORK_DATA:
			std::cerr << numServerMessages++ << "SEND OBJ DATA" << std::endl;
			break;
		default:
			std::cerr << numServerMessages++ << "Server send message of type " << msgType << ", don't know what to do with it..." << std::endl;
		}

		if (response.forClient == -1)
		{
			for (auto& cid : clientIDs)
			{
				perClient[cid].push_back(response);
			}

			//ServerNetwork::broadcastBytes(response.body, response.messageType, response.id);
		}
		else
		{
			perClient[response.forClient].push_back(response);
			//ServerNetwork::sendBytes(response.forClient, response.body, response.messageType, response.id);
		}
	}

	// FOR EVERYTHING ELSE
	for (auto& response : NetworkManager::postbox)
	{
		if (response.messageType == CREATE_OBJECT_NETWORK_DATA ||
			response.messageType == DESTROY_OBJECT_NETWORK_DATA)
		{
			continue;
		}

		int& msgType = response.messageType;

		switch (msgType)
		{
		case CLIENTS_CONN_NETWORK_DATA:
			std::cerr << numServerMessages++ << "SEND ALL CLEAR MESSAGE AFTER SERVER INITIALIZATION" << std::endl;
			break;
		case TRANSFORM_NETWORK_DATA:
			// std::cerr << numClientMessages++ << "RECV TRANSFORM DATA" << std::endl;
			break;
		case MESH_NETWORK_DATA:
			std::cerr << numServerMessages++ << "SEND MESH DATA" << std::endl;
			break;
		case CAMERA_NETWORK_DATA:
			std::cerr << numServerMessages++ << "SEND CAMERA DATA" << std::endl;
			break;
		case LIGHT_NETWORK_DATA:
			std::cerr << numServerMessages++ << "SEND LIGHT DATA" << std::endl;
			break;
		case CREATE_OBJECT_NETWORK_DATA:
		case DESTROY_OBJECT_NETWORK_DATA:
			std::cerr << numServerMessages++ << "SEND OBJ DATA" << std::endl;
			break;
		default:
			std::cerr << numServerMessages++ << "Server sent message of type " << msgType << ", don't know what to do with it..." << std::endl;
		}

		if (response.forClient == -1)
		{
			for (auto& cid : clientIDs)
			{
				perClient[cid].push_back(response);
			}

			//ServerNetwork::broadcastBytes(response.body, response.messageType, response.id);
		}
		else
		{
			perClient[response.forClient].push_back(response);
			//ServerNetwork::sendBytes(response.forClient, response.body, response.messageType, response.id);
		}
	}

	for (auto& clibox : perClient)
	{
		ServerNetwork::sendMessages(clibox.first, clibox.second);
	}

	NetworkManager::postbox.clear();
}

// TODO allow the sender to force all messages to be resolved, not just the latest one
void NetworkManager::PostMessage(const std::vector<char>& bytes, int messageType, int messageID, int forClient)
{
	if (NetworkManager::state != SERVER_MODE) return;

	auto key = std::make_pair(messageType, messageID);
	postbox.push_back(NetworkResponse(messageType, messageID, bytes, forClient));
}

// --- CLIENT FUNC --- //

std::tuple<std::vector<ClientID>, ClientID> NetworkManager::InitializeClient(std::string serverIP, std::string port)
{
	if (NetworkManager::state != UNINITIALIZED)
	{
		throw std::runtime_error("Cannot initialize initialized server");
	}
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
				std::cerr << "Wrong message received before all clear..." << std::endl;
			}
		}
	}

	// and when we're all done
	GameObject::AddPreFixedUpdateCallback(NetworkManager::ReceiveClientMessages);
	GameObject::AddPostFixedUpdateCallback(NetworkManager::SendClientMessages);

	NetworkManager::state = CLIENT_MODE;
	return std::make_tuple(NetworkManager::clientIDs, NetworkManager::myClientID);
}

static int numClientMessages = 0;
void NetworkManager::ReceiveClientMessages()
{
	if (NetworkManager::state != CLIENT_MODE)
	{
		throw std::runtime_error("Cannot receive messages on unitialized network");
	}

	std::vector<NetworkResponse> receivedMessages = ClientNetwork::receiveMessages();
	for (auto& received : receivedMessages)
	{
		int& msgType = received.messageType;

		switch (msgType)
		{
		case CLIENTS_CONN_NETWORK_DATA:
			std::cerr << numClientMessages++ << "RECv ALL CLEAR MESSAGE AFTER SERVER INITIALIZATION" << std::endl;
			break;
		case TRANSFORM_NETWORK_DATA:
			// std::cerr << numClientMessages++ << "RECV TRANSFORM DATA" << std::endl;
			Transform::Dispatch(received.body, received.messageType, received.id);
			break;
		case MESH_NETWORK_DATA:
			std::cerr << numClientMessages++ << "RECV MESH DATA" << std::endl;
			Mesh::Dispatch(received.body, received.messageType, received.id);
			break;
		case CAMERA_NETWORK_DATA:
			std::cerr << numClientMessages++ << "RECV CAMERA DATA" << std::endl;
			Camera::Dispatch(received.body, received.messageType, received.id);
			break;
		case LIGHT_NETWORK_DATA:
			std::cerr << numClientMessages++ << "RECV LIGHT DATA" << std::endl;
			Light::Dispatch(received.body, received.messageType, received.id);
			break;
		case CREATE_OBJECT_NETWORK_DATA:
		case DESTROY_OBJECT_NETWORK_DATA:
			std::cerr << numClientMessages++ << "RECV OBJ DATA" << std::endl;
			GameObject::Dispatch(received.body, received.messageType, received.id);
			break;
		default:
			std::cerr << numClientMessages++ << "Client received message of type " << msgType << ", don't know what to do with it..." << std::endl;
		}
	}
}

void NetworkManager::SendClientMessages()
{
	if (NetworkManager::state != CLIENT_MODE)
	{
		throw std::runtime_error("Cannot send messages on unitialized network");
	}

	std::vector<char> bytes = Input::serialize(NetworkManager::myClientID);

	if (!std::equal(bytes.begin(), bytes.end(), NetworkManager::lastBytesSent.begin(), NetworkManager::lastBytesSent.end())) {
		NetworkManager::lastBytesSent.resize(bytes.size());
		std::copy(bytes.begin(), bytes.end(), NetworkManager::lastBytesSent.begin());
		ClientNetwork::sendBytes(bytes, INPUT_NETWORK_DATA, NetworkManager::myClientID);
	}
}

// --- OFFLINE FUNC -- //

void NetworkManager::InitializeOffline()
{
	if (NetworkManager::state != UNINITIALIZED)
	{
		throw std::runtime_error("Cannot initialize intialized server");
	}

	NetworkManager::state = OFFLINE;
}

void NetworkManager::attachCameraTo(ClientID client, int gameObjectID) {
	std::cout << "Sending request to client " << client << " to attach mainCamera to object " << gameObjectID << std::endl;
	NetworkManager::PostMessage(std::vector<char>(), CAMERA_NETWORK_DATA, gameObjectID, client);
}
