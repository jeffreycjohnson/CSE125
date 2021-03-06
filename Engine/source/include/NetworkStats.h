#pragma once
#include <unordered_map>
#include <vector>

class NetworkStats
{

private:
	static bool debug;
	static std::unordered_map<int, int> clientBytesSent;
	static std::unordered_map<int, std::vector<unsigned long>> clientMsgTypeSent;
	static std::vector<int> clients;
public:
	NetworkStats();
	~NetworkStats();
	
	static void registerClients(std::vector<int> clients);
	static void registerMsgType(int clientId, int msgType);
	static void registerBytesSent(int clientID, int numBytes);
	static void clearClientBytes();
	static void checkDebugButton();
};

