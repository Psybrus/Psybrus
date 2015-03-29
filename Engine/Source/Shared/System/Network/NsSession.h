#pragma once

#include "Events/EvtPublisher.h"

#include "System/Network/NsForwardDeclarations.h"

#include <thread>

/**
 * @brief States the session can be in.
 */
enum class NsSessionState
{
	/// Currently in disconnected state.
	DISCONNECTED,
	/// Connecting to other session.
	CONNECTING,
	/// Ready to communicate.
	READY,
	/// Disconnecting from remote session.
	DISCONNECTING,
	/// Error.
	ERROR
};

/**
 * @brief Type of network session.
 */
enum class NsSessionType
{
	/// This session is the server.
	SERVER,
	/// This session is a client.
	CLIENT,
};

/**
 * @brief Network session. 
 * Stores information about who is in a session, and ability to connect to other
 * remote sessions.
 */
class NsSession:
	public EvtPublisher
{
public:
	enum Client { CLIENT };
	enum Server { SERVER };

public:
	NsSession( Client, const std::string& Address, BcU16 Port );
	NsSession( Server, BcU32 MaxClients, BcU16 Port );

	virtual ~NsSession();

private:
	void workerThread();


private:
	RakNet::RakPeerInterface* PeerInterface_;
	NsSessionType Type_;
	std::atomic< int > Active_;
	std::atomic< NsSessionState > State_;
	std::thread WorkerThread_;


};
