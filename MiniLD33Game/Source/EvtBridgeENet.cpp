/**************************************************************************
*
* File:		EvtBridgeENet.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event bridge over ENet.
*		
*		
*
* 
**************************************************************************/

#include "EvtBridgeENet.h"

#include <stun.h>

////////////////////////////////////////////////////////////////////////////////
// Ctor
EvtBridgeENet::EvtBridgeENet( EvtPublisher* pPublisher ):
	EvtBridge( pPublisher )
{
	if( enet_initialize() != 0 )
	{
		BcPrintf( "EvtBridgeENet: An error occurred initializing ENet.\n" );
	}

	pServerHost_ = NULL;	
	pClientHost_ = NULL;	
	pPeer_ = NULL;

	NoofHosts_ = 0;
	IsConnected_ = BcFalse;
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
EvtBridgeENet::~EvtBridgeENet()
{
	enet_deinitialize();
}

////////////////////////////////////////////////////////////////////////////////
// connect
BcBool EvtBridgeENet::connect( BcU32 ClientID, BcU32 RemoteAddress, BcU16 RemotePort, BcU16 LocalPort, int ServerSocketFileDesc )
{
	ClientID_ = ClientID;

	BcPrintf( "EvtBridgeENet RemoteAddr:%u.%u.%u.%u:%u/LocalPort:%u/ServerSock:%u\n", 
		( RemoteAddress >> 24 ) & 0xff,
		( RemoteAddress >> 16 ) & 0xff,
		( RemoteAddress >> 8 ) & 0xff,
		( RemoteAddress ) & 0xff,
		RemotePort,
		LocalPort,
		ServerSocketFileDesc
		);
	
	// Setup a server host.
	ServerAddress_.host = ENET_HOST_ANY;
	ServerAddress_.port = LocalPort;
	pServerHost_ = enet_host_create( &ServerAddress_ /* the address to bind the server host to */, 
		                                1               /* allow up to 2 clients and/or outgoing connections */,
		                                2               /* allow up to 2 channels to be used, 0 and 1 */,
		                                0               /* assume any amount of incoming bandwidth */,
		                                0,               /* assume any amount of outgoing bandwidth */
										ServerSocketFileDesc );
	
	ServerAddress_.host = htonl( RemoteAddress ); // lol hax
	ServerAddress_.port = RemotePort;
	pClientHost_ = enet_host_create ( NULL /* create a client host */,
									  1 /* only allow 1 outgoing connection */,
									  2 /* allow up 2 channels to be used, 0 and 1 */,
									  57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
									  14400 / 8, /* 56K modem with 14 Kbps upstream bandwidth */ 
									  0 );
		
	if( pServerHost_ && pClientHost_ )
	{
		// Get a peer from client host.
		pPeer_ = enet_host_connect( pClientHost_, &ServerAddress_, 2, 0 );

		pPeer_->data = "Local client";

		if( pPeer_ == NULL )
		{
			BcPrintf( "Client (%u) An error has occurred creating peer.\n", ClientID );
			BcBreakpoint;
		}
		else
		{
			BcTimer Timer;
			Timer.mark();
			while( Timer.time() < 20.0f )
			{
				serviceHost( pServerHost_, 1000 );
				serviceHost( pClientHost_, 1000 );

				if( NoofHosts_ == 2 )
				{
					BcPrintf( "Client (%u) Connected to server.\n", ClientID );
					IsConnected_ = BcTrue;
					break;
				}
			}

			if( NoofHosts_ != 2 )
			{
				BcPrintf( "Client (%u) An error has occurred.\n", ClientID );
			}
		}
	}
	
	return IsConnected_;
}

////////////////////////////////////////////////////////////////////////////////
// update
BcBool EvtBridgeENet::update( BcReal Delta )
{
	serviceHost( pServerHost_ );
	serviceHost( pClientHost_ );

	// need 2 hosts.
	return IsConnected_ && NoofHosts_ == 2;
}

////////////////////////////////////////////////////////////////////////////////
// bridge
//virtual
void EvtBridgeENet::bridge( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize )
{
	BcAssert( IsConnected_ && NoofHosts_ == 2 );	
	
	ENetPacket* pPacket = enet_packet_create( &ID, sizeof( ID ), ENET_PACKET_FLAG_RELIABLE );

	enet_packet_resize( pPacket, sizeof( ID ) + EventSize );
	BcMemCopy( &pPacket->data[ sizeof( ID ) ], &EventBase, EventSize );

	enet_host_broadcast( pServerHost_, 0, pPacket );

	/*
	if( ClientID_ == 0 )
	{
		enet_host_broadcast( pHost_, 0, pPacket );
	}
	else
	{
		enet_peer_send( pPeer_, 0, pPacket );
	}
	*/
}

////////////////////////////////////////////////////////////////////////////////
// serviceHost
//virtual
void EvtBridgeENet::serviceHost( ENetHost* pHost, BcU32 Timeout )
{
	ENetEvent Event;

	const BcChar* pHostType = pHost == pServerHost_ ? "ServerHost:" : "ClientHost";

	if( pHost )
	{
		while( enet_host_service ( pHost, &Event, Timeout ) > 0 )
		{
			BcPrintf( "(%s) ENet Event: %u\n", pHostType, Event.type );
			switch (Event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				{
					BcPrintf ("(%s) A new client connected from %x:%u.\n", pHostType, 
							Event.peer -> address.host,
							Event.peer -> address.port);

					/* Store any relevant client information here. */
					Event.peer -> data = "Client information";

					++NoofHosts_;
				}
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				{
					EvtID InID = *reinterpret_cast< EvtID* >( &Event.packet -> data[ 0 ] );
					EvtBaseEvent& InEvent = *reinterpret_cast< EvtBaseEvent* >( &Event.packet -> data[ sizeof( InID ) ] );
					publish( InID, InEvent, Event.packet -> dataLength - sizeof( InID ) );

					BcPrintf( "(%s) Publishing event 0x%x. %u bytes.\n", pHostType, InID, Event.packet -> dataLength - sizeof( InID ) );

					/* Clean up the packet now that we're done using it. */
					enet_packet_destroy ( Event.packet );
				}
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				{
					BcPrintf ("(%s) %s disconected.\n", pHostType, Event.peer -> data);
				
					/* Reset the peer's client information. */
				
					Event.peer -> data = NULL;

					--NoofHosts_;
				}
				break;
			}
		}
	}
}
