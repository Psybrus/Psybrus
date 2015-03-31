#include "System/Network/NsSessionImpl.h"
#include "System/Network/NsPlayerImpl.h"

#include "MessageIdentifiers.h"
#include "RakPeer.h"
#include "BitStream.h"

#include <boost/format.hpp>

//////////////////////////////////////////////////////////////////////////
// NsSessionMessageID
enum class NsSessionMessageID : BcU8
{
	PLAYER_REQUEST_JOIN = ID_USER_PACKET_ENUM,
	PLAYER_ACCEPT_JOIN,
	PLAYER_DATA
};

//////////////////////////////////////////////////////////////////////////
// NsSessionMessageChannel
enum class NsSessionMessageChannel : BcU8
{
	/// Reserve 0-127 for user.

	/// Player messages.
	PLAYER = 128,
};

//////////////////////////////////////////////////////////////////////////
// Player IDs.
static const BcU32 CLIENT_START_PLAYER_ID = 0x00000000;
static const BcU32 SERVER_START_PLAYER_ID = 0x80000000;
static_assert( CLIENT_START_PLAYER_ID < SERVER_START_PLAYER_ID, "Client ID must be less than server ID." );

//////////////////////////////////////////////////////////////////////////
// Ctor
NsSessionImpl::NsSessionImpl( Client, const std::string& Address, BcU16 Port ) :
	PeerInterface_( RakNet::RakPeerInterface::GetInstance() ),
	Type_( NsSessionType::CLIENT ),
	Active_( 1 ),
	State_( NsSessionState::DISCONNECTED )
{
	PSY_LOGSCOPEDCATEGORY( boost::str( boost::format( "NsSession %1%" ) % this ) );
	PSY_LOG( "Starting worker thread, and trying to connect to server." );

	RakNet::SocketDescriptor Desc;
	PeerInterface_->Startup( 1, &Desc, 1 );
	PeerInterface_->Connect( Address.c_str(), Port, nullptr, 0 );

	WorkerThread_ = std::thread( std::bind( &NsSessionImpl::workerThread, this ) );

	PlayerIDCounter_.store( CLIENT_START_PLAYER_ID );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
NsSessionImpl::NsSessionImpl( Server, BcU32 MaxClients, BcU16 Port ) :
	PeerInterface_( RakNet::RakPeerInterface::GetInstance() ),
	Type_( NsSessionType::SERVER ),
	Active_( 1 ),
	State_( NsSessionState::DISCONNECTED )
{
	PSY_LOGSCOPEDCATEGORY( boost::str( boost::format( "NsSession %1%" ) % this ) );
	PSY_LOG( "Starting worker thread, and trying to start server." );

	RakNet::SocketDescriptor Desc( Port, 0 );
	PeerInterface_->Startup( MaxClients, &Desc, 1 );
	PeerInterface_->SetMaximumIncomingConnections( MaxClients );

	WorkerThread_ = std::thread( std::bind( &NsSessionImpl::workerThread, this ) );

	PlayerIDCounter_.store( SERVER_START_PLAYER_ID );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
NsSessionImpl::~NsSessionImpl()
{
	Active_.store( 0 );
	WorkerThread_.join();
	RakNet::RakPeerInterface::DestroyInstance( PeerInterface_ );
}

//////////////////////////////////////////////////////////////////////////
// createPlayer
//virtual
class NsPlayer* NsSessionImpl::createPlayer()
{
	auto Player = new NsPlayerImpl( this );
	Player->setID( allocPlayerID() );

	std::lock_guard< std::mutex > Lock( PlayerLock_ );
	Players_.push_back( Player );

	return Player;
}

//////////////////////////////////////////////////////////////////////////
// workerThread
void NsSessionImpl::workerThread()
{
	PSY_LOGSCOPEDCATEGORY( boost::str( boost::format( "NsSession %1%" ) % this ) );
	PSY_LOG( "Starting to receive packets." );

	OwningThread_ = BcCurrentThreadId();

	RakNet::BitStream BitStream;

	while( Active_ )
	{
		// Handle players.
		{
			std::lock_guard< std::mutex > Lock( PlayerLock_ );
			for( auto Player : Players_ )
			{
				switch( Player->getState() )
				{
				case NsPlayerState::NOT_IN_SESSION:
					{
					}
					break;
				case NsPlayerState::JOINING_SESSION:
					{
					}
					break;
				case NsPlayerState::IN_SESSION:
					{
					}
					break;
				case NsPlayerState::LEAVING_SESSION:
					{
					}
					break;
				}
			}
		}

		// Handle packets.
		for( auto Packet = PeerInterface_->Receive();
			 Packet != nullptr;
			 PeerInterface_->DeallocatePacket( Packet ), Packet = PeerInterface_->Receive() )
		{
			PSY_LOG( "Received packet:" );
			PSY_LOGSCOPEDINDENT;

			auto PacketId = Packet->data[ 0 ];
			switch( PacketId )
			{
			case ID_CONNECTION_REQUEST_ACCEPTED:
				PSY_LOG( "ID_CONNECTION_REQUEST_ACCEPTED" );
				break;

			case ID_CONNECTION_ATTEMPT_FAILED:
				PSY_LOG( "ID_CONNECTION_ATTEMPT_FAILED" );
				break;

			case ID_ALREADY_CONNECTED:
				PSY_LOG( "ID_ALREADY_CONNECTED" );
				break;

			case ID_NEW_INCOMING_CONNECTION:
				PSY_LOG( "ID_NEW_INCOMING_CONNECTION" );
				break;

			case ID_NO_FREE_INCOMING_CONNECTIONS:
				PSY_LOG( "ID_NO_FREE_INCOMING_CONNECTIONS" );
				break;

			case ID_CONNECTION_LOST:
				PSY_LOG( "ID_CONNECTION_LOST" );
				break;

			case ID_CONNECTION_BANNED:
				PSY_LOG( "ID_CONNECTION_BANNED" );
				break;

			case ID_INVALID_PASSWORD:
				PSY_LOG( "ID_INVALID_PASSWORD" );
				break;

			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
				PSY_LOG( "ID_INCOMPATIBLE_PROTOCOL_VERSION" );
				break;

			case ID_IP_RECENTLY_CONNECTED:
				PSY_LOG( "ID_IP_RECENTLY_CONNECTED" );
				break;

			case ID_TIMESTAMP:
				PSY_LOG( "ID_TIMESTAMP" );
				break;

			case ID_UNCONNECTED_PONG:
				PSY_LOG( "ID_UNCONNECTED_PONG" );
				break;

			case ID_ADVERTISE_SYSTEM:
				PSY_LOG( "ID_ADVERTISE_SYSTEM" );
				break;

			case ID_DOWNLOAD_PROGRESS:
				PSY_LOG( "ID_DOWNLOAD_PROGRESS" );
				break;

			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
				PSY_LOG( "ID_REMOTE_DISCONNECTION_NOTIFICATION" );
				break;

			case ID_REMOTE_CONNECTION_LOST:
				PSY_LOG( "ID_REMOTE_CONNECTION_LOST" );
				break;

			case ID_REMOTE_NEW_INCOMING_CONNECTION:
				PSY_LOG( "ID_REMOTE_NEW_INCOMING_CONNECTION" );
				break;

			default:
				PSY_LOG( "Unknown" );
				break;
			}
		}

		// Sleep for little.
		BcSleep( 0.005f );
	}
}

//////////////////////////////////////////////////////////////////////////
// allocPlayerID
BcU32 NsSessionImpl::allocPlayerID()
{
	auto AllocatedID = PlayerIDCounter_++;

	if( Type_ == NsSessionType::SERVER )
	{
		BcAssert( AllocatedID >= SERVER_START_PLAYER_ID );
	}
	else if( Type_ == NsSessionType::CLIENT )
	{
		BcAssert( AllocatedID >= CLIENT_START_PLAYER_ID && AllocatedID < SERVER_START_PLAYER_ID );
	}

	return AllocatedID;
}

//////////////////////////////////////////////////////////////////////////
// sendPlayerJoinRequest
void NsSessionImpl::sendPlayerJoinRequest( NsPlayer* Player )
{
	BcAssert( Type_ == NsSessionType::CLIENT );

	RakNet::BitStream BitStream;
	BitStream.Write( (BcU8)NsSessionMessageID::PLAYER_REQUEST_JOIN );
	BitStream.Write( Player->getID() );

	PeerInterface_->Send( 
		&BitStream, IMMEDIATE_PRIORITY, RELIABLE_ORDERED, (char)NsSessionMessageChannel::PLAYER,
		RakNet::AddressOrGUID(), false, 0 );
}

//////////////////////////////////////////////////////////////////////////
// receivePlayerRequestJoin
void NsSessionImpl::receivePlayerRequestJoin( const void* Data, size_t Size )
{
	BcAssert( Type_ == NsSessionType::SERVER );

	NsSessionMessageID MessageID;
	RakNet::BitStream BitStream( (unsigned char*)Data, Size, false );

	BitStream.Read( MessageID );
	BcAssert( MessageID == NsSessionMessageID::PLAYER_ACCEPT_JOIN );
}

//////////////////////////////////////////////////////////////////////////
// sendPlayerAcceptJoin
void NsSessionImpl::sendPlayerAcceptJoin( const RakNet::AddressOrGUID& Dest, class NsPlayer* Player )
{
	BcAssert( Type_ == NsSessionType::SERVER );

	RakNet::BitStream BitStream;
	BitStream.Write( (BcU8)NsSessionMessageID::PLAYER_ACCEPT_JOIN );
	BitStream.Write( Player->getID() );

	PeerInterface_->Send( 
		&BitStream, IMMEDIATE_PRIORITY, RELIABLE_ORDERED, (char)NsSessionMessageChannel::PLAYER,
		Dest, false, 0 );
}

//////////////////////////////////////////////////////////////////////////
// receivePlayerAcceptJoin
void NsSessionImpl::receivePlayerAcceptJoin( const void* Data, size_t Size )
{
	BcAssert( Type_ == NsSessionType::CLIENT );

	NsSessionMessageID MessageID;
	BcU32 PlayerID = 0;
	RakNet::BitStream BitStream( (unsigned char*)Data, Size, false );

	BitStream.Read( MessageID );
	BcAssert( MessageID == NsSessionMessageID::PLAYER_ACCEPT_JOIN );

	BitStream >> PlayerID;
}

//////////////////////////////////////////////////////////////////////////
// sendPlayerData
void NsSessionImpl::sendPlayerData( class NsPlayer* Player )
{
	RakNet::BitStream BitStream;
	BitStream.Write( (BcU8)NsSessionMessageID::PLAYER_DATA );
	BitStream.Write( Player->getID() );

	// TODO: Player data.
	//BitStream.Write( Player->getName().c_str() );

	PeerInterface_->Send( 
		&BitStream, IMMEDIATE_PRIORITY, RELIABLE_ORDERED, (char)NsSessionMessageChannel::PLAYER,
		RakNet::AddressOrGUID(), true, 0 );
}

//////////////////////////////////////////////////////////////////////////
// receivePlayerData
void NsSessionImpl::receivePlayerData( const void* Data, size_t Size )
{
	NsSessionMessageID MessageID;
	BcU32 PlayerID = 0;
	std::string PlayerName;
	RakNet::BitStream BitStream( (unsigned char*)Data, Size, false );

	BitStream.Read( MessageID );
	BcAssert( MessageID == NsSessionMessageID::PLAYER_DATA );

	BitStream >> PlayerID;
	BitStream >> PlayerName;
}
