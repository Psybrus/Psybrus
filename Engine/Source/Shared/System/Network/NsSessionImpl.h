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
	/**
	 * Create a client session.
	 * @param Address Address of server, IPv4 or IPv6 (test this).
	 * @param Port Port of server.
	 */
	NsSessionImpl( Client, const std::string& Address, BcU16 Port );

	/**
	 * Create a server session.
	 * @param Address MaxClients Maximum number of clients supported.
	 * @param Port Port of server.
	 * @param Advertise Should we advertise server by broadcasting?
	 */
	NsSessionImpl( Server, BcU32 MaxClients, BcU16 Port, bool Advertise );
	virtual ~NsSessionImpl();

	BcU32 getNoofRemoteSessions() const override;
	NsGUID getRemoteGUIDByIndex( BcU32 Index ) override;

	void send( 
		NsGUID RemoteGUID, BcU8 Channel, const void* Data, size_t DataSize, 
		NsPriority Priority, NsReliability Reliability ) override;

	void broadcast( 
		BcU8 Channel, const void* Data, size_t DataSize, 
		NsPriority Priority, NsReliability Reliability ) override;

	BcBool registerMessageHandler( BcU8 Channel, NsSessionMessageHandler* Handler ) override;
	BcBool deregisterMessageHandler( BcU8 Channel, NsSessionMessageHandler* Handler ) override;

private:
	void workerThread();


private:
	RakNet::RakPeerInterface* PeerInterface_;
	RakNet::ConnectionGraph2* ConnectionGraph_;
	NsSessionType Type_;
	BcU16 Port_;
	std::atomic< int > Active_;
	std::atomic< NsSessionState > State_;
	bool Advertise_;

	BcThreadId OwningThread_;
	std::thread WorkerThread_;
	SysFence CallbackFence_;

	std::mutex HandlerLock_;
	std::array< NsSessionMessageHandler*, 256 > MessageHandlers_;
};
