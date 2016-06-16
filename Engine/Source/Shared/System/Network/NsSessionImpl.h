#pragma once

#include "Events/EvtPublisher.h"

#include "System/Network/NsForwardDeclarations.h"
#include "System/Network/NsSession.h"
#include "System/SysFence.h"

#include <set>
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
	 */
	NsSessionImpl( Client, NsSessionHandler* Handler );

	/**
	 * Create a client session.
	 * @param Address Address of server, IPv4 or IPv6 (test this).
	 * @param Port Port of server.
	 */
	NsSessionImpl( Client, NsSessionHandler* Handler, const char* Address, BcU16 Port );

	/**
	 * Create a server session.
	 * @param Name of server.
	 * @param MaxClients Maximum number of clients supported.
	 * @param Port Port of server. 
	 * @param AdvertisePort If > 0, we advertise server by broadcasting using this port.
	 */
	NsSessionImpl( Server, NsSessionHandler* Handler, const char* Name, BcU32 MaxClients, BcU16 Port, BcU16 AdvertisePort );
	virtual ~NsSessionImpl();

	size_t getNoofRemoteSessions() const override;
	NsGUID getRemoteGUIDByIndex( size_t Index ) override;
	size_t getClientSessions( NsGUID* OutGUIDs, size_t MaxGUIDs ) const override;

	void send( 
		NsGUID RemoteGUID, BcU8 Channel, const void* Data, size_t DataSize, 
		NsPriority Priority, NsReliability Reliability ) override;

	void broadcast( 
		BcU8 Channel, const void* Data, size_t DataSize, 
		NsPriority Priority, NsReliability Reliability ) override;


	bool registerMessageHandler( BcU8 Channel, NsSessionMessageHandler* Handler ) override;
	bool deregisterMessageHandler( BcU8 Channel, NsSessionMessageHandler* Handler ) override;

private:
	void workerThread();

	void addClient( NsGUID GUID );
	void removeClient( NsGUID GUID );

private:
	RakNet::RakPeerInterface* PeerInterface_;
	NsSessionHandler* Handler_;
	NsSessionType Type_;
	std::array< char, 128 > Name_;
	BcU32 MaxClients_;
	BcU16 Port_;
	std::atomic< int > Active_;
	std::atomic< NsSessionState > State_;
	BcU16 AdvertisePort_;

	BcThreadId OwningThread_;
	std::thread WorkerThread_;
	SysFence CallbackFence_;

	std::mutex HandlerLock_;
	std::array< NsSessionMessageHandler*, 256 > MessageHandlers_;

	mutable std::mutex ClientsLock_;
	std::set< NsGUID > Clients_;
};
