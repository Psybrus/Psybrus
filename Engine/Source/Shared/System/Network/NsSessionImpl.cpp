#include "System/Network/NsSessionImpl.h"
#include "System/SysKernel.h"

#include "Base/BcString.h"

#if !PLATFORM_HTML5 && !PLATFORM_WINPHONE

#include "MessageIdentifiers.h"
#include "RakPeer.h"
#include "ConnectionGraph2.h"
#include "BitStream.h"

#include <algorithm>

//////////////////////////////////////////////////////////////////////////
// NsSessionMessageID
enum class NsSessionMessageID : BcU8
{
	/// Request broadcast from server.
	SESSION_BROADCAST_REQUEST = ID_USER_PACKET_ENUM,

	/// Message.
	SESSION_MESSAGE
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
// GUID conversion.
NsGUID FromRakNet( RakNet::RakNetGUID GUID )
{
	return GUID.g;
}

RakNet::RakNetGUID ToRakNet( NsGUID GUID )
{
	return RakNet::RakNetGUID( GUID );
}


//////////////////////////////////////////////////////////////////////////
// Ctor
NsSessionImpl::NsSessionImpl( Client, NsSessionHandler* Handler ) :
	PeerInterface_( RakNet::RakPeerInterface::GetInstance() ),
	ConnectionGraph_( RakNet::ConnectionGraph2::GetInstance() ),
	Handler_( Handler ),
	Type_( NsSessionType::CLIENT ),
	MaxClients_( 0 ),
	Port_( 0 ),
	Active_( 1 ),
	State_( NsSessionState::DISCONNECTED ),
	AdvertisePort_( 0 )
{
	PSY_LOGSCOPEDCATEGORY( NsSession );
	PSY_LOG( "Starting worker thread, and trying to connect to server." );

	RakNet::SocketDescriptor Desc;
	PeerInterface_->AttachPlugin( ConnectionGraph_ );
	PeerInterface_->Startup( 1, &Desc, 1 );

	MessageHandlers_.fill( nullptr );

	WorkerThread_ = std::thread( std::bind( &NsSessionImpl::workerThread, this ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
NsSessionImpl::NsSessionImpl( Client, NsSessionHandler* Handler, const std::string& Address, BcU16 Port ) :
	PeerInterface_( RakNet::RakPeerInterface::GetInstance() ),
	ConnectionGraph_( RakNet::ConnectionGraph2::GetInstance() ),
	Handler_( Handler ),
	Type_( NsSessionType::CLIENT ),
	MaxClients_( 0 ),
	Port_( Port ),
	Active_( 1 ),
	State_( NsSessionState::DISCONNECTED ),
	AdvertisePort_( 0 )
{
	PSY_LOGSCOPEDCATEGORY( NsSession );
	PSY_LOG( "Starting worker thread, and trying to connect to server." );

	RakNet::SocketDescriptor Desc;
	PeerInterface_->AttachPlugin( ConnectionGraph_ );
	PeerInterface_->Startup( 1, &Desc, 1 );
	PeerInterface_->Connect( Address.c_str(), Port, nullptr, 0 );

	MessageHandlers_.fill( nullptr );

	WorkerThread_ = std::thread( std::bind( &NsSessionImpl::workerThread, this ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
NsSessionImpl::NsSessionImpl( Server, NsSessionHandler* Handler, BcU32 MaxClients, BcU16 Port, BcU16 AdvertisePort ) :
	PeerInterface_( RakNet::RakPeerInterface::GetInstance() ),
	ConnectionGraph_( RakNet::ConnectionGraph2::GetInstance() ),
	Handler_( Handler ),
	Type_( NsSessionType::SERVER ),
	MaxClients_( MaxClients ),
	Port_( Port ),
	Active_( 1 ),
	State_( NsSessionState::DISCONNECTED ),
	AdvertisePort_( AdvertisePort )
{
	PSY_LOGSCOPEDCATEGORY( NsSession );
	PSY_LOG( "Starting worker thread, and trying to start server." );

	RakNet::SocketDescriptor Desc( Port, 0 );
	PeerInterface_->AttachPlugin( ConnectionGraph_ );
	PeerInterface_->Startup( std::max( MaxClients, BcU32( 1 ) ), &Desc, 1 );
	PeerInterface_->SetMaximumIncomingConnections( static_cast< unsigned short >( MaxClients ) );

	MessageHandlers_.fill( nullptr );

	WorkerThread_ = std::thread( std::bind( &NsSessionImpl::workerThread, this ) );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
NsSessionImpl::~NsSessionImpl()
{
	Active_.store( 0 );
	CallbackFence_.wait();
	WorkerThread_.join();
	PeerInterface_->DetachPlugin( ConnectionGraph_ );
	RakNet::ConnectionGraph2::DestroyInstance( ConnectionGraph_ );
	RakNet::RakPeerInterface::DestroyInstance( PeerInterface_ );
}

//////////////////////////////////////////////////////////////////////////
// getNoofRemoteSessions
BcU32 NsSessionImpl::getNoofRemoteSessions() const
{
	return PeerInterface_->NumberOfConnections();
}

//////////////////////////////////////////////////////////////////////////
// getRemoteGUIDByIndex
NsGUID NsSessionImpl::getRemoteGUIDByIndex( BcU32 Index )
{
	return FromRakNet( PeerInterface_->GetGUIDFromIndex( Index ) );
}

//////////////////////////////////////////////////////////////////////////
// send
void NsSessionImpl::send( 
		NsGUID RemoteGUID, BcU8 Channel, const void* Data, size_t DataSize, 
		NsPriority Priority, NsReliability Reliability )
{
	// TODO: Optimise this entire thing.
	RakNet::BitStream BitStream;

	auto RemoteSystemAddress = PeerInterface_->GetSystemAddressFromGuid( ToRakNet( RemoteGUID ) );
	BitStream.Write( (BcU8)NsSessionMessageID::SESSION_MESSAGE );
	BitStream.Write( Channel );
	BitStream.Serialize( true, (char*)Data, DataSize );
	PeerInterface_->Send( 
		(const char*)BitStream.GetData(), BitStream.GetNumberOfBytesUsed(),
		(PacketPriority)Priority, 
		(PacketReliability)Reliability, Channel, 
		RemoteSystemAddress,
		true, 0 );
}

//////////////////////////////////////////////////////////////////////////
// broadcast
void NsSessionImpl::broadcast( 
		BcU8 Channel, const void* Data, size_t DataSize, 
		NsPriority Priority, NsReliability Reliability )
{
	// TODO: Optimise this entire thing.
	RakNet::BitStream BitStream;
	if( Type_ == NsSessionType::SERVER )
	{
		BitStream.Write( (BcU8)NsSessionMessageID::SESSION_MESSAGE );
		BitStream.Write( Channel );
		BitStream.Serialize( true, (char*)Data, DataSize );
		PeerInterface_->Send( 
			(const char*)BitStream.GetData(), BitStream.GetNumberOfBytesUsed(),
			(PacketPriority)Priority, 
			(PacketReliability)Reliability, Channel, 
			RakNet::UNASSIGNED_SYSTEM_ADDRESS,
			true, 0 );
	}
	else
	{
		BitStream.Write( (BcU8)NsSessionMessageID::SESSION_BROADCAST_REQUEST );
		BitStream.Write( Channel );
		BitStream.Serialize( true, (char*)Data, DataSize );
		PeerInterface_->Send( 
			(const char*)BitStream.GetData(), BitStream.GetNumberOfBytesUsed(),
			(PacketPriority)Priority, 
			(PacketReliability)Reliability, Channel, 
			RakNet::UNASSIGNED_SYSTEM_ADDRESS,
			true, 0 );
	}
}

//////////////////////////////////////////////////////////////////////////
// registerMessageHandler
bool NsSessionImpl::registerMessageHandler( BcU8 Channel, NsSessionMessageHandler* Handler )
{
	std::lock_guard< std::mutex > Lock( HandlerLock_ );
	auto CanSet = MessageHandlers_[ Channel ] == nullptr;
	BcAssert( CanSet );
	if( CanSet )
	{
		MessageHandlers_[ Channel ] = Handler;
	}
	return CanSet;
}

//////////////////////////////////////////////////////////////////////////
// deregisterMessageHandler
bool NsSessionImpl::deregisterMessageHandler( BcU8 Channel, NsSessionMessageHandler* Handler )
{
	std::lock_guard< std::mutex > Lock( HandlerLock_ );
	auto CanUnset = MessageHandlers_[ Channel ] == Handler;
	BcAssert( CanUnset );
	if( CanUnset )
	{
		MessageHandlers_[ Channel ] = nullptr;
	}
	return CanUnset;
}

//////////////////////////////////////////////////////////////////////////
// workerThread
void NsSessionImpl::workerThread()
{
	PSY_LOGSCOPEDCATEGORY( NsSession );
	PSY_LOG( "Starting to receive packets." );

	OwningThread_ = BcCurrentThreadId();

	RakNet::BitStream BitStream;

	BcTimer AdvertiseTimer;
	AdvertiseTimer.mark();

	while( Active_ )
	{
		// Handle packets.
		for( auto Packet = PeerInterface_->Receive();
			 Packet != nullptr;
			 Packet = PeerInterface_->Receive() )
		{
			BcU8 PacketId = Packet->data[ 0 ];
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

			case ID_CONNECTED_PONG:
				PSY_LOG( "ID_CONNECTED_PONG" );
				break;

			case ID_UNCONNECTED_PONG:
				PSY_LOG( "ID_UNCONNECTED_PONG" );
				break;

			case ID_ADVERTISE_SYSTEM:
				{
					CallbackFence_.increment();
					SysKernel::pImpl()->enqueueCallback( [ this, Packet ]()
						{
							std::array< char, 128 > NameBuffer = { 0 }; 
							BcStrCopy( NameBuffer.data(), std::min( NameBuffer.size(), size_t( Packet->length ) ), reinterpret_cast< char* >( Packet->data + 1 ) );
							Handler_->onAdvertisedSystem( NameBuffer.data(), 
								Packet->systemAddress.ToString( false ),
								Packet->systemAddress.GetPort(),
								static_cast< BcU32 >( -1 ) );
							PeerInterface_->DeallocatePacket( Packet );
							CallbackFence_.decrement();
						} );
					continue;
				}
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

			case (BcU8)NsSessionMessageID::SESSION_BROADCAST_REQUEST:
				{
					//PSY_LOG( "NsSessionMessageID::SESSION_BROADCAST_REQUEST" );
					if( Type_ == NsSessionType::SERVER )
					{
						Packet->data[ 0 ] = (BcU8)NsSessionMessageID::SESSION_MESSAGE;
						PeerInterface_->Send( 
							(const char*)Packet->data, Packet->length,
							HIGH_PRIORITY, RELIABLE_ORDERED, 0,
							RakNet::UNASSIGNED_SYSTEM_ADDRESS,
							true, 0 );
						// Need to send to self too.
						PeerInterface_->SendLoopback( 
							(const char*)Packet->data, Packet->length );
					}
				}
				break;

			case (BcU8)NsSessionMessageID::SESSION_MESSAGE:
				{
					//PSY_LOG( "NsSessionMessageID::SESSION_MESSAGE" );
					BcU8 Channel = Packet->data[ 1 ];
					auto MessageHandler = MessageHandlers_[ Channel ];

					// If we have a hander, invoke the message received on the main thread.
					if( MessageHandler != nullptr )
					{
						CallbackFence_.increment();
						SysKernel::pImpl()->enqueueCallback( [ this, MessageHandler, Packet ]()
							{
								BcAssert( MessageHandler );
								BcAssert( Packet );
								BcAssert( Packet->length > 2 );
								MessageHandler->onMessageReceived( &Packet->data[ 2 ], Packet->length - 2 );
								PeerInterface_->DeallocatePacket( Packet );
								CallbackFence_.decrement();
							} );
						continue;
					}
				}	
				break;

			default:
				PSY_LOG( "Unknown: %u", BcU32( PacketId ) );
				break;
			}

			// Deallocate packet.
			PeerInterface_->DeallocatePacket( Packet );
		}

		// Sleep for little.
		BcSleep( 0.005f );

		// Advertise system.
		// TEST CODE. PROPER SYSTEM REQUIRED LATER.
		if( AdvertisePort_ > 0 )
		{
			const BcF32 AdvertiseTime = 5.0f;
			const char* AdvertiseAddress = "255.255.255.255"; // IPv6!?
			if( AdvertiseTimer.time() > AdvertiseTime )
			{
				const char* AdvertiseData = "THIS IS A SERVER";

				PeerInterface_->AdvertiseSystem( AdvertiseAddress, AdvertisePort_, AdvertiseData, BcStrLength( AdvertiseData ) );
				AdvertiseTimer.mark();
			}
		}
	}
}

#endif // !PLATFORM_HTML5 && !PLATFORM_WINPHONE
