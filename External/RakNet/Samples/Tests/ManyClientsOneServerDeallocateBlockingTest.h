#pragma once

#include "TestInterface.h"

#include "RakString.h"

#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakPeer.h"
#include "RakSleep.h"
#include "RakNetTime.h"
#include "GetTime.h"
#include "DebugTools.h"
#include "CommonFunctions.h"

using namespace RakNet;
class ManyClientsOneServerDeallocateBlockingTest :
	public TestInterface
{
public:
	ManyClientsOneServerDeallocateBlockingTest(void);
	~ManyClientsOneServerDeallocateBlockingTest(void);
	int RunTest(DataStructures::List<RakString> params,bool isVerbose,bool noPauses);//should return 0 if no error, or the error number
	RakString GetTestName();
	RakString ErrorCodeToString(int errorCode);
	void DestroyPeers();

protected:
	void WaitForConnectionRequestsToComplete(RakPeerInterface **peerList, int peerNum, bool isVerbose);
	void WaitAndPrintResults(RakPeerInterface **peerList, int peerNum, bool isVerbose,RakPeerInterface *server);
private:
	static const int clientNum= 256;
	RakPeerInterface *clientList[clientNum];//A list of clients
	RakPeerInterface *server;
};
