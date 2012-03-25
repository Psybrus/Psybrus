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
BcBool EvtBridgeENet::connect( BcU32 ClientID, const BcChar* pAddress, BcU16 Port )
{
	ClientID_ = ClientID;

	/*
	// Get STUN server address.
	StunAddress4 StunAddress;
	StunAddress4 LocalAddress;
	if( stunParseServerName( "stunserver.org", StunAddress ) )
	{
		// Set port.
		int Port = 6000 + ClientID_;

		// Get NAT type.
		bool PreservePort = 0;
		bool HairPin = 0;
		NatType Type = stunNatType( StunAddress, true,  &PreservePort, &HairPin , Port, 0 );
		
		UInt32 OutIP;
		UInt16 OutPort;
		stunParseHostName( "localhost", OutIP, OutPort, Port );

		int a = 0; ++a;
	}
	*/
	
	// Setup a server host.
	ServerAddress_.host = ENET_HOST_ANY;
	ServerAddress_.port = 6000 + ClientID;
	pServerHost_ = enet_host_create( &ServerAddress_ /* the address to bind the server host to */, 
		                                1               /* allow up to 2 clients and/or outgoing connections */,
		                                2               /* allow up to 2 channels to be used, 0 and 1 */,
		                                0               /* assume any amount of incoming bandwidth */,
		                                0               /* assume any amount of outgoing bandwidth */ );

	enet_address_set_host( &ServerAddress_, pAddress );
	ServerAddress_.port = 6000 + ( 1 - ClientID );
	pClientHost_ = enet_host_create ( NULL /* create a client host */,
									  1 /* only allow 1 outgoing connection */,
									  2 /* allow up 2 channels to be used, 0 and 1 */,
									  57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
									  14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */ );
		
	if( pServerHost_ && pClientHost_ )
	{
		// Get a peer from client host.
		pPeer_ = enet_host_connect( pClientHost_, &ServerAddress_, 2, 0 );

		if( pPeer_ == NULL )
		{
			BcPrintf( "(%u) An error has occurred.\n", ClientID );
			BcBreakpoint;
		}
		else
		{
			BcTimer Timer;
			Timer.mark();
			while( Timer.time() < 10.0f )
			{
				serviceHost( pServerHost_ );
				serviceHost( pClientHost_ );

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
void EvtBridgeENet::serviceHost( ENetHost* pHost )
{
	ENetEvent Event;

	if( pHost )
	{
		while( enet_host_service ( pHost, &Event, 0 ) > 0 )
		{
			switch (Event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				{
					BcPrintf ("A new client connected from %x:%u.\n", 
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

					/* Clean up the packet now that we're done using it. */
					enet_packet_destroy ( Event.packet );
				}
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				{
					BcPrintf ("%s disconected.\n", Event.peer -> data);
				
					/* Reset the peer's client information. */
				
					Event.peer -> data = NULL;

					--NoofHosts_;
				}
				break;
			}
		}
	}
}
