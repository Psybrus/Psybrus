/**************************************************************************
*
* File:		EvtBridgeRakNet.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event bridge over RakNet.
*		
*		
*
* 
**************************************************************************/

#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakNetTypes.h"  // MessageID

#include "EvtBridgeRakNet.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
EvtBridgeRakNet::EvtBridgeRakNet( EvtPublisher* pPublisher ):
	EvtBridge( pPublisher )
{
	pPeer_ = RakNet::RakPeerInterface::GetInstance();
	pNatPunchthroughClient_ = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
EvtBridgeRakNet::~EvtBridgeRakNet()
{
	if( pNatPunchthroughClient_ != NULL )
	{
		pPeer_->DetachPlugin( pNatPunchthroughClient_ );
		RakNet::NatPunchthroughClient::DestroyInstance( pNatPunchthroughClient_ );
	}

	RakNet::RakPeerInterface::DestroyInstance( pPeer_ );

	pNatPunchthroughClient_ = NULL;
	pPeer_ = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// connect
BcBool EvtBridgeRakNet::connect( BcU32 ClientID, BcU32 RemoteAddress, BcU16 RemotePort, BcU16 LocalPort, int ServerSocketFileDesc )
{
	ClientID_ = ClientID;

	BcPrintf( "EvtBridgeRakNet RemoteAddr:%u.%u.%u.%u:%u/LocalPort:%u/ServerSock:%u\n", 
		( RemoteAddress >> 24 ) & 0xff,
		( RemoteAddress >> 16 ) & 0xff,
		( RemoteAddress >> 8 ) & 0xff,
		( RemoteAddress ) & 0xff,
		RemotePort,
		LocalPort,
		ServerSocketFileDesc
		);

	
	if( ClientID_ == 0 )
	{
		SocketDesc_ = RakNet::SocketDescriptor( LocalPort, "" );
		pPeer_->Startup( 1, &SocketDesc_, 1 );
		pPeer_->SetMaximumIncomingConnections( 1 );
	}
	else
	{
		BcChar RemoteAddressString[ 128 ];
		BcSPrintf( RemoteAddressString, "%u.%u.%u.%u",
		           ( RemoteAddress >> 24 ) & 0xff,
		           ( RemoteAddress >> 16 ) & 0xff,
		           ( RemoteAddress >> 8 ) & 0xff,
		           ( RemoteAddress ) & 0xff );
		pPeer_->Startup( 1, &SocketDesc_, 1 );
		pPeer_->Connect( RemoteAddressString, RemotePort, 0, 0 );
	}
	
	return BcTrue;
}

////////////////////////////////////////////////////////////////////////////////
// connectNAT
BcBool EvtBridgeRakNet::connectNAT( BcU32 ClientID, BcU64 RemoteGUID )
{
	ClientID_ = ClientID;

	pNatPunchthroughClient_ = RakNet::NatPunchthroughClient::GetInstance();
	pPeer_->AttachPlugin( pNatPunchthroughClient_ );

	/*
	SystemAddress ServerAddress( );
	pNatPunchthroughClient_->OpenNAT( RemoteGUID, ServerAddress );
	*/

	/*
	if( ClientID_ == 0 )
	{
		SocketDesc_ = RakNet::SocketDescriptor( LocalPort, "" );
		pPeer_->Startup( 1, &SocketDesc_, 1 );
		pPeer_->SetMaximumIncomingConnections( 1 );
	}
	else
	{
		BcChar RemoteAddressString[ 128 ];
		BcSPrintf( RemoteAddressString, "%u.%u.%u.%u",
		           ( RemoteAddress >> 24 ) & 0xff,
		           ( RemoteAddress >> 16 ) & 0xff,
		           ( RemoteAddress >> 8 ) & 0xff,
		           ( RemoteAddress ) & 0xff );
		pPeer_->Startup( 1, &SocketDesc_, 1 );
		pPeer_->Connect( RemoteAddressString, RemotePort, 0, 0 );
	}
	*/
	
	return BcTrue;
}

////////////////////////////////////////////////////////////////////////////////
// update
BcBool EvtBridgeRakNet::update( BcReal Delta )
{
	for( RakNet::Packet* pPacket = pPeer_->Receive(); pPacket; pPeer_->DeallocatePacket( pPacket ), pPacket = pPeer_->Receive() )
	{
		switch( pPacket->data[0] )
		{
			// Normal peer messages.
			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
				BcPrintf("EvtBridgeRakNet(%u) Another client has disconnected.\n", ClientID_);
				break;
			case ID_REMOTE_CONNECTION_LOST:
				BcPrintf("EvtBridgeRakNet(%u) Another client has lost the connection.\n", ClientID_);
				break;
			case ID_REMOTE_NEW_INCOMING_CONNECTION:
				BcPrintf("EvtBridgeRakNet(%u) Another client has connected.\n", ClientID_);
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
				BcPrintf("EvtBridgeRakNet(%u) Our connection request has been accepted.\n", ClientID_);
				break;					
			case ID_NEW_INCOMING_CONNECTION:
				BcPrintf("EvtBridgeRakNet(%u) A connection is incoming.\n", ClientID_);
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				BcPrintf("EvtBridgeRakNet(%u) The server is full.\n", ClientID_);
				break;
			case ID_DISCONNECTION_NOTIFICATION:
				BcPrintf("EvtBridgeRakNet(%u) We have been disconnected.\n", ClientID_ );
				break;
			case ID_CONNECTION_LOST:
				BcPrintf("EvtBridgeRakNet(%u) Connection lost.\n", ClientID_ );
				break;
			case ID_CONNECTION_ATTEMPT_FAILED:
				BcPrintf("EvtBridgeRakNet(%u) Connection attempt failed.\n", ClientID_ );
				break;

			// NAT punchthrough messages.
			case ID_NAT_PUNCHTHROUGH_SUCCEEDED:
				BcPrintf("EvtBridgeRakNet(%u) NAT punchthrough succeeded.\n", ClientID_ );
				break;

				// Others.
			default:
				BcPrintf("EvtBridgeRakNet(%u) Message with identifier %i has arrived.\n", ClientID_, pPacket->data[0]);

				// It's an event.
				if( pPacket->data[0] == ID_USER_PACKET_ENUM + 1 )
				{
					EvtID InID =  BcBigEndian( *reinterpret_cast< EvtID* >( &pPacket->data[ 1 ] ) );
					EvtBaseEvent& InEvent = *reinterpret_cast< EvtBaseEvent* >( &pPacket->data[ 1 + sizeof( InID ) ] );
					publish( InID, InEvent, pPacket->length - sizeof( InID ) );
				}
				break;
		}
	}

	// Need to have a connection open (only 1 other player.)
	return pPeer_->NumberOfConnections() == 1;
}

////////////////////////////////////////////////////////////////////////////////
// bridge
//virtual
void EvtBridgeRakNet::bridge( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize )
{
	RakNet::BitStream BitStreamOut;

	BcU32 GameMessageID = ID_USER_PACKET_ENUM + 1;

	BitStreamOut.Write( (RakNet::MessageID)GameMessageID );
	BitStreamOut.Write( ID );
	BitStreamOut.Write( (const char*)&EventBase, EventSize );

	RakNet::SystemAddress NullAddr;
	pPeer_->Send( &BitStreamOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, NullAddr, true );
}
