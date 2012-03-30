#pragma once


#include "RakString.h"

#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakPeer.h"
#include "RakSleep.h"
#include "RakNetTime.h"
#include "GetTime.h"
#include "DebugTools.h"
	#include "RakTimer.h"

using namespace RakNet;
class CommonFunctions
{
public:
	CommonFunctions(void);
	~CommonFunctions(void);

	static bool WaitAndConnect(RakPeerInterface *peer,char* ip,unsigned short int port,int millisecondsToWait);
	static bool WaitForMessageWithID(RakPeerInterface *reciever,int id,int millisecondsToWait);
	static Packet * WaitAndReturnMessageWithID(RakPeerInterface *reciever,int id,int millisecondsToWait);
	static void DisconnectAndWait(RakPeerInterface *peer,char* ip,unsigned short int port);
    static bool ConnectionStateMatchesOptions(RakPeerInterface *peer, SystemAddress currentSystem, bool isConnected, bool isConnecting=false, bool isPending=false, bool isDisconnecting=false, bool isNotConnected=false, bool isLoopBack=false, bool isSilentlyDisconnecting=false);
};
