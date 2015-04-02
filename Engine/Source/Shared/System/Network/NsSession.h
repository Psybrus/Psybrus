#pragma once

#include "Events/EvtPublisher.h"

#include "System/Network/NsTypes.h"
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
// Data handlers.
class NsSessionMessageHandler
{
public:
	NsSessionMessageHandler(){}
	virtual ~NsSessionMessageHandler(){}

	virtual void onMessageReceived( const void* Data, size_t DataSize ) = 0;
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
	 * Broadcast to all players.
	 * @param Channel Channel to send on. Messages ordered by channel.
	 * @param Data Pointer to data. Only needs to be valid for call.
	 * @param DataSize Data size.
	 * @param Priority Priority to send message. See NsPriority.
	 * @param Reliability Reliability to send with. See NsReliability.
	 */
	virtual void broadcast( 
		BcU8 Channel, const void* Data, size_t DataSize, 
		NsPriority Priority, NsReliability Reliability ) = 0;

	/**
	 * Register message handler.
	 * @return Successfully registered.
	 */
	virtual BcBool registerMessageHandler( BcU8 Channel, NsSessionMessageHandler* Handler ) = 0;

	/**
	 * Deregister message handler.
	 * @return Successfully deregistered.
	 */
	virtual BcBool deregisterMessageHandler( BcU8 Channel, NsSessionMessageHandler* Handler ) = 0;

};
