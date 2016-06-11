#pragma once

#include "Events/EvtPublisher.h"

#include "System/Network/NsTypes.h"
#include "System/Network/NsForwardDeclarations.h"

//////////////////////////////////////////////////////////////////////////
// Undef ERROR...sigh
#ifdef ERROR
#undef ERROR
#endif

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
// Session handler.
class NsSessionHandler
{
public:
	NsSessionHandler(){}
	virtual ~NsSessionHandler(){}

	/**
	 * Called when an advertised system is visible.
	 * @param Name Name of remote system as c-string.
	 * @param Address Address of remote system as c-string.
	 * @param Port Port of remote system.
	 * @param Ping Ping of remote system in ms.
	 */
	virtual void onAdvertisedSystem( const char* Name, const char* Address, BcU16 Port, BcU32 Ping ) = 0;

	/**
	 * Called when connection has been accepted.
	 */
	virtual void onConnectionAccepted() = 0;

	/**
	 * Called when connection has failed.
	 */
	virtual void onConnectionFailed() = 0;

	/**
	 * Called when a system's connection is lost.
	 * @param SystemGUID Remote system GUID that lost connection. Can be server or client.
	 */
	virtual void onSystemConnectionLost( NsGUID SystemGUID ) = 0;

	/**
	 * Called when system disconnects.
	 * @param SessionGUID Remote system GUID that was disconnected. Can be server or client.
	 */
	virtual void onSystemDisconnect( NsGUID SystemGUID ) = 0;

	/**
	 * Called when system connects.
	 * @param SessionGUID Remote system GUID that connected. Can be server or client.
	 */
	virtual void onSystemConnected( NsGUID SystemGUID ) = 0;
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
	 * Get number of remote sessions.
	 */
	virtual size_t getNoofRemoteSessions() const = 0;

	/**
	 * Get GUID of remote session by index.
	 */
	virtual NsGUID getRemoteGUIDByIndex( size_t Index ) = 0;

	/**
	 * Get number of client sessions.
	 * @param OutGUIDs Point to output GUIDs.
	 * @param MaxGUIDs Maximum number of GUIDs that can be filled in.
	 * @return Number of GUIDs set.
	 */
	virtual size_t getClientSessions( NsGUID* OutGUIDs, size_t MaxGUIDs ) const = 0;
	
	/**
	 * Send to specific remote session.
	 * @param RemoteGUID GUID of remote session to send to.
	 * @param Channel Channel to send on. Messages ordered by channel.
	 * @param Data Pointer to data. Only needs to be valid for call.
	 * @param DataSize Data size.
	 * @param Priority Priority to send message. See NsPriority.
	 * @param Reliability Reliability to send with. See NsReliability.
	 */
	virtual void send( 
		NsGUID RemoteGUID, BcU8 Channel, const void* Data, size_t DataSize, 
		NsPriority Priority, NsReliability Reliability ) = 0;

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
	virtual bool registerMessageHandler( BcU8 Channel, NsSessionMessageHandler* Handler ) = 0;

	/**
	 * Deregister message handler.
	 * @return Successfully deregistered.
	 */
	virtual bool deregisterMessageHandler( BcU8 Channel, NsSessionMessageHandler* Handler ) = 0;
};
