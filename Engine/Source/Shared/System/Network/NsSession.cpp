#include "System/Network/NsSession.h"


#include "MessageIdentifiers.h"
#include "RakPeer.h"

NsSession::NsSession( Client, const std::string& Address, BcU16 Port ) :
	PeerInterface_( RakNet::RakPeerInterface::GetInstance() ),
	Type_( NsSessionType::CLIENT ),
	Active_( 1 ),
	State_( NsSessionState::DISCONNECTED )
{
	RakNet::SocketDescriptor Desc;
	PeerInterface_->Startup( 1, &Desc, 1 );
	PeerInterface_->Connect( Address.c_str(), Port, nullptr, 0 );

	WorkerThread_ = std::thread( std::bind( &NsSession::workerThread, this ) );
}


NsSession::NsSession( Server, BcU32 MaxClients, BcU16 Port ) :
	PeerInterface_( RakNet::RakPeerInterface::GetInstance() ),
	Type_( NsSessionType::SERVER ),
	Active_( 1 ),
	State_( NsSessionState::DISCONNECTED )
{
	RakNet::SocketDescriptor Desc( Port, 0 );
	PeerInterface_->Startup( MaxClients, &Desc, 1 );
	PeerInterface_->SetMaximumIncomingConnections( MaxClients );


	WorkerThread_ = std::thread( std::bind( &NsSession::workerThread, this ) );
}


NsSession::~NsSession()
{
	Active_.store( 0 );
	WorkerThread_.join();
	RakNet::RakPeerInterface::DestroyInstance( PeerInterface_ );
}


void NsSession::workerThread()
{
	PSY_LOGSCOPEDCATEGORY( "NsSession" );

	while( Active_ )
	{
		for( auto Packet = PeerInterface_->Receive();
			 Packet != nullptr;
			 PeerInterface_->DeallocatePacket( Packet ), Packet = PeerInterface_->Receive() )
		{
			auto PacketId = Packet->data[ 0 ];
			switch( PacketId )
			{
			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
				PSY_LOG( "Received ID_REMOTE_DISCONNECTION_NOTIFICATION" );
				break;

			case ID_REMOTE_CONNECTION_LOST:
				PSY_LOG( "Received ID_REMOTE_CONNECTION_LOST" );
				break;

			case ID_REMOTE_NEW_INCOMING_CONNECTION:
				PSY_LOG( "Received ID_REMOTE_NEW_INCOMING_CONNECTION" );
				break;

			case ID_CONNECTION_REQUEST_ACCEPTED:
				PSY_LOG( "Received ID_CONNECTION_REQUEST_ACCEPTED" );
				break;

			case ID_NEW_INCOMING_CONNECTION:
				PSY_LOG( "Received ID_NEW_INCOMING_CONNECTION" );
				break;

			case ID_NO_FREE_INCOMING_CONNECTIONS:
				PSY_LOG( "Received ID_NO_FREE_INCOMING_CONNECTIONS" );
				break;

			case ID_DISCONNECTION_NOTIFICATION:
				PSY_LOG( "Received ID_DISCONNECTION_NOTIFICATION" );
				break;

			case ID_CONNECTION_LOST:
				PSY_LOG( "Received ID_CONNECTION_LOST" );
				break;
			}
		}
	}
}
