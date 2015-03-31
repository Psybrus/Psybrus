#pragma once

#include "Events/EvtPublisher.h"

#include "System/Network/NsForwardDeclarations.h"

//////////////////////////////////////////////////////////////////////////
// States the session can be in.
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

//////////////////////////////////////////////////////////////////////////
// Type of network session.
enum class NsSessionType
{
	/// This session is the server.
	SERVER,
	/// This session is a client.
	CLIENT,
};

//////////////////////////////////////////////////////////////////////////
// @brief Network session. 
// Stores information about who is in a session, and ability to connect to other
// remote sessions.
class NsSession:
	public EvtPublisher
{
public:
	NsSession();
	virtual ~NsSession();

	/**
	 * Create player to be in session.
	 * @return New player object.
	 */
	virtual class NsPlayer* createPlayer() = 0;
};
