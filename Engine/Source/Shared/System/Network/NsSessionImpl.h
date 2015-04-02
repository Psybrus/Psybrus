#pragma once

#include "Events/EvtPublisher.h"

#include "System/Network/NsForwardDeclarations.h"
#include "System/Network/NsSession.h"
#include "System/SysFence.h"

#include <thread>

//////////////////////////////////////////////////////////////////////////
// @brief Network session. 
// Stores information about who is in a session, and ability to connect to other
// remote sessions.
class NsSessionImpl:
	public NsSession
{
public:
	enum Client { CLIENT };
	enum Server { SERVER };

public:
	NsSessionImpl( Client, const std::string& Address, BcU16 Port );
	NsSessionImpl( Server, BcU32 MaxClients, BcU16 Port );
	virtual ~NsSessionImpl();

	void broadcast( 
		BcU8 Channel, const void* Data, size_t DataSize, 
		NsPriority Priority, NsReliability Reliability ) override;

	BcBool registerMessageHandler( BcU8 Channel, NsSessionMessageHandler* Handler ) override;
	BcBool deregisterMessageHandler( BcU8 Channel, NsSessionMessageHandler* Handler ) override;

private:
	void workerThread();


private:
	RakNet::RakPeerInterface* PeerInterface_;
	NsSessionType Type_;
	std::atomic< int > Active_;
	std::atomic< NsSessionState > State_;
	BcThreadId OwningThread_;
	std::thread WorkerThread_;
	SysFence CallbackFence_;

	std::mutex HandlerLock_;
	std::array< NsSessionMessageHandler*, 256 > MessageHandlers_;
};
