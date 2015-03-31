#pragma once

#include "Events/EvtPublisher.h"

#include "System/Network/NsForwardDeclarations.h"
#include "System/Network/NsSession.h"

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

	virtual class NsPlayer* createPlayer() override;

private:
	void workerThread();

	BcU32 allocPlayerID();

	void sendPlayerJoinRequest( NsPlayer* Player );
	void receivePlayerRequestJoin( const void* Data, size_t Size );

	void sendPlayerAcceptJoin( const RakNet::AddressOrGUID& Dest, class NsPlayer* Player );
	void receivePlayerAcceptJoin( const void* Data, size_t Size );

	void sendPlayerData( class NsPlayer* Player );
	void receivePlayerData( const void* Data, size_t Size );

private:
	RakNet::RakPeerInterface* PeerInterface_;
	NsSessionType Type_;
	std::atomic< int > Active_;
	std::atomic< NsSessionState > State_;
	BcThreadId OwningThread_;
	std::thread WorkerThread_;

	// Players
	std::mutex PlayerLock_;
	std::vector< class NsPlayerImpl* > Players_;
	std::atomic< BcU32 > PlayerIDCounter_;
};
