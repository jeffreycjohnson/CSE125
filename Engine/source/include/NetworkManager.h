#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "ForwardDecs.h"
#include "NetworkUtility.h"
#include "NetworkStruct.h"
#include "Texture.h"

#include <map>
#include <utility>
#include <vector>
#include <tuple>

typedef int ClientID;

enum NetworkState
{
	UNINITIALIZED,
	INITIALIZING,
	SHUTDOWN,
	CLIENT_MODE,
	SERVER_MODE,
	OFFLINE
};


class NetworkManager
{
private:
	static NetworkState state;
	static Texture *loadingScreen;

	static std::vector<ClientID> clientIDs;
	static ClientID myClientID;

	static std::vector<NetworkResponse> postbox;

	static void ReceiveServerMessages();
	static void SendServerMessages();

	static void ReceiveClientMessages();
	static void SendClientMessages();
	static std::vector<char> lastBytesSent;
	static bool gameStarted, waiting;

public:
	NetworkManager();
	~NetworkManager();

	static std::vector<ClientID> InitializeServer(std::string port, int numberOfClients);
	static std::tuple<std::vector<ClientID>, ClientID> InitializeClient(std::string serverIP, std::string port);
	static void InitializeOffline();

	static void attachCameraTo(ClientID client, int gameObjectID);

	static void PostMessage(const std::vector<char>& bytes, MessageType messageType, int messageID, int forClient = -1);

	static NetworkState getState();
	static void setState(NetworkState newState);

	static void drawUI();
};

#endif // NETWORK_MANAGER_H