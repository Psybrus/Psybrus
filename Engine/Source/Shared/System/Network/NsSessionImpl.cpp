#include "System/Network/NsSessionImpl.h"
#include "System/SysKernel.h"

#include "Base/BcString.h"

#if !PLATFORM_HTML5 && !PLATFORM_WINPHONE

#include "MessageIdentifiers.h"
#include "RakPeer.h"
#include "ConnectionGraph2.h"
#include "BitStream.h"

#include <algorithm>
#include <cinttypes>

//////////////////////////////////////////////////////////////////////////
// NsSessionMessageID
enum class NsSessionMessageID : BcU8
{
	/// Request broadcast from server.
	SESSION_BROADCAST_REQUEST = ID_USER_PACKET_ENUM,

	// Add client.
	SESSION_REMOTE_CLIENT_ADD,

	// Remove client.
	SESSION_REMOTE_CLIENT_REMOVE,

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
	PeerInterface_->Startup( 1, &Desc, 1 );

	MessageHandlers_.fill( nullptr );

	WorkerThread_ = std::thread( std::bind( &NsSessionImpl::workerThread, this ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
NsSessionImpl::NsSessionImpl( Client, NsSessionHandler* Handler, const char* Address, BcU16 Port ) :
	PeerInterface_( RakNet::RakPeerInterface::GetInstance() ),
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

	Name_.fill( 0 );

	RakNet::SocketDescriptor Desc;
	PeerInterface_->Startup( 1, &Desc, 1 );
	PeerInterface_->Connect( Address, Port, nullptr, 0 );

	MessageHandlers_.fill( nullptr );

	WorkerThread_ = std::thread( std::bind( &NsSessionImpl::workerThread, this ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
NsSessionImpl::NsSessionImpl( Server, NsSessionHandler* Handler, const char* Name, BcU32 MaxClients, BcU16 Port, BcU16 AdvertisePort ) :
	PeerInterface_( RakNet::RakPeerInterface::GetInstance() ),
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

	BcStrCopy( Name_.data(), (BcU32)Name_.size(), Name );

	RakNet::SocketDescriptor Desc( Port, 0 );
	while( PeerInterface_->Startup( std::max( MaxClients, BcU32( 1 ) ), &Desc, 1 ) == RakNet::SOCKET_PORT_ALREADY_IN_USE )
	{
		Desc.port++;
	};

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
	PeerInterface_->Shutdown( 100, 0, HIGH_PRIORITY );
	RakNet::RakPeerInterface::DestroyInstance( PeerInterface_ );
}

//////////////////////////////////////////////////////////////////////////
// getNoofRemoteSessions
size_t NsSessionImpl::getNoofRemoteSessions() const
{
	return static_cast< size_t >( PeerInterface_->NumberOfConnections() );
}

//////////////////////////////////////////////////////////////////////////
// getRemoteGUIDByIndex
NsGUID NsSessionImpl::getRemoteGUIDByIndex( size_t Index )
{
	return FromRakNet( PeerInterface_->GetGUIDFromIndex( static_cast< BcU32 >( Index ) ) );
}

//////////////////////////////////////////////////////////////////////////
// getClientSessions
size_t NsSessionImpl::getClientSessions( NsGUID* OutGUIDs, size_t MaxGUIDs ) const
{
	std::lock_guard< std::mutex > Lock( ClientsLock_ );

	if( OutGUIDs )
	{
		auto It = Clients_.begin();
		for( size_t Idx = 0; Idx < std::min( MaxGUIDs, (size_t)Clients_.size() ); ++Idx )
		{
			OutGUIDs[ Idx ] = *It++;
		}
	}

	return Clients_.size();
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
	BitStream.Serialize( true, (char*)Data, (BcU32)DataSize );
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
		BitStream.Serialize( true, (char*)Data, (BcU32)DataSize );
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
		BitStream.Serialize( true, (char*)Data, (BcU32)DataSize );
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
				CallbackFence_.increment();
				SysKernel::pImpl()->enqueueCallback( [ this, Packet ]()
					{
						Handler_->onConnectionAccepted();
						PeerInterface_->DeallocatePacket( Packet );
						CallbackFence_.decrement();
					} );
				continue;

			case ID_CONNECTION_ATTEMPT_FAILED:
				CallbackFence_.increment();
				SysKernel::pImpl()->enqueueCallback( [ this, Packet ]()
					{
						Handler_->onConnectionFailed();
						PeerInterface_->DeallocatePacket( Packet );
						CallbackFence_.decrement();
					} );
				continue;

			case ID_DISCONNECTION_NOTIFICATION:
				{
					NsGUID GUID = FromRakNet( Packet->guid );
					removeClient( GUID );

					if( Type_ == NsSessionType::CLIENT )
					{
						CallbackFence_.increment();
						SysKernel::pImpl()->enqueueCallback( [ this, GUID ]()
							{
								Handler_->onSystemConnectionLost( GUID );
								CallbackFence_.decrement();
							} );
					}
				}
				continue;

			case ID_CONNECTION_LOST:
				{
					NsGUID GUID = FromRakNet( Packet->guid );
					removeClient( GUID );

					if( Type_ == NsSessionType::CLIENT )
					{
						CallbackFence_.increment();
						SysKernel::pImpl()->enqueueCallback( [ this, GUID ]()
							{
								Handler_->onSystemDisconnect( GUID );
								CallbackFence_.decrement();
							} );
					}
				}
				continue;
				
			case ID_NEW_INCOMING_CONNECTION:
				{
					NsGUID GUID = FromRakNet( Packet->guid );
					addClient( GUID );
				}
				break;

			case ID_ALREADY_CONNECTED:
				PSY_LOG( "ID_ALREADY_CONNECTED" );
				break;

			case ID_NO_FREE_INCOMING_CONNECTIONS:
				PSY_LOG( "ID_NO_FREE_INCOMING_CONNECTIONS" );
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
							BcStrCopy( NameBuffer.data(), std::min( (BcU32)NameBuffer.size(), (BcU32)Packet->length - 1 ), reinterpret_cast< char* >( Packet->data + 1 ) );
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

			case (BcU8)NsSessionMessageID::SESSION_REMOTE_CLIENT_ADD:
				{
					std::lock_guard< std::mutex > Lock( ClientsLock_ );
					BcU8 NumGUIDs = Packet->data[ 1 ];
					auto* ClientGUID = reinterpret_cast< NsGUID* >( Packet->data + 2 );
					for( BcU32 Idx = 0; Idx < NumGUIDs; ++Idx )
					{
						auto GUID = *ClientGUID++;
						if( Clients_.find( GUID ) == Clients_.end() )
						{
							CallbackFence_.increment();
							SysKernel::pImpl()->enqueueCallback( [ this, GUID ]()
								{
									Handler_->onSystemConnected( GUID );
									CallbackFence_.decrement();
								} );

							Clients_.insert( GUID );
						}						
					}
#if PSY_DEBUG
					PSY_LOG( "%u clients added. Clients in session:", BcU32( NumGUIDs ) );
					auto It = Clients_.begin();
					for( size_t Idx = 0; Idx < Clients_.size(); ++Idx )
					{
						PSY_LOG( " - Client %u: %" PRIu64, Idx, *It++ );
					}
#endif
				}
				break;

			case (BcU8)NsSessionMessageID::SESSION_REMOTE_CLIENT_REMOVE:
				{
					std::lock_guard< std::mutex > Lock( ClientsLock_ );
					BcU8 NumGUIDs = Packet->data[ 1 ];
					auto* ClientGUID = reinterpret_cast< NsGUID* >( Packet->data + 2 );
					for( BcU32 Idx = 0; Idx < NumGUIDs; ++Idx )
					{
						auto GUID = *ClientGUID++;
						if( Clients_.find( GUID ) != Clients_.end() )
						{
							CallbackFence_.increment();
							SysKernel::pImpl()->enqueueCallback( [ this, GUID ]()
								{
									Handler_->onSystemDisconnect( GUID );
									CallbackFence_.decrement();
								} );
						}
						Clients_.erase( GUID );
					}
#if PSY_DEBUG
					PSY_LOG( "%u clients removed. Clients in session:", BcU32( NumGUIDs ) );
					auto It = Clients_.begin();
					for( size_t Idx = 0; Idx < Clients_.size(); ++Idx )
					{
						PSY_LOG( " - Client %u: %" PRIu64, Idx, *It++ );
					}
#endif
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

		// Advertise system.
		// TEST CODE. PROPER SYSTEM REQUIRED LATER.
		if( AdvertisePort_ > 0 )
		{
			const BcF32 AdvertiseTime = 5.0f;
			const char* AdvertiseAddress = "255.255.255.255"; // IPv6!?
			if( AdvertiseTimer.time() > AdvertiseTime )
			{
				// Send to 8 ports. Local testing.
				for( BcU16 Idx = 0; Idx < 8; ++Idx )
				{
					PeerInterface_->AdvertiseSystem( AdvertiseAddress, AdvertisePort_ + Idx, Name_.data(), BcStrLength( Name_.data() ) + 1 );
				}
				AdvertiseTimer.mark();
			}
		}

		// Sleep for little.
		BcSleep( 0.005f );
	}
}

//////////////////////////////////////////////////////////////////////////
// addClient
void NsSessionImpl::addClient( NsGUID GUID )
{
	if( Type_ == NsSessionType::SERVER )
	{
		std::lock_guard< std::mutex > Lock( ClientsLock_ );
		Clients_.insert( GUID );

		const size_t Size = 1 + 1 + ( sizeof( NsGUID ) * Clients_.size() );
		std::array< char, 1024 > ClientAddPacket;
		BcAssert( Size <= ClientAddPacket.size() );
		ClientAddPacket[ 0 ] = (BcU8)NsSessionMessageID::SESSION_REMOTE_CLIENT_ADD;
		ClientAddPacket[ 1 ] = static_cast< BcU8 >( Clients_.size() );
		NsGUID* OutGUIDs = reinterpret_cast< NsGUID* >( &ClientAddPacket[ 2 ] );
		auto It = Clients_.begin();
		for( size_t Idx = 0; Idx < Clients_.size(); ++Idx )
		{
			NsGUID InGUID = *It++;
			memcpy( OutGUIDs++, &InGUID, sizeof( InGUID ) );
		}

		PeerInterface_->Send(
			ClientAddPacket.data(), Size,
			HIGH_PRIORITY, RELIABLE_ORDERED, 0,
			RakNet::UNASSIGNED_SYSTEM_ADDRESS,
			true, 0 );

		PeerInterface_->SendLoopback( ClientAddPacket.data(), ClientAddPacket.size() );
	}
}

//////////////////////////////////////////////////////////////////////////
// removeClient
void NsSessionImpl::removeClient( NsGUID GUID )
{
	if( Type_ == NsSessionType::SERVER )
	{
		const size_t Size = 1 + 1 + sizeof( NsGUID );
		std::array< char, Size > ClientRemovePacket;
		ClientRemovePacket[ 0 ] = (BcU8)NsSessionMessageID::SESSION_REMOTE_CLIENT_REMOVE;
		ClientRemovePacket[ 1 ] = 1;
		memcpy( &ClientRemovePacket[ 2 ], &GUID, sizeof( GUID ) );
		PeerInterface_->Send(
			ClientRemovePacket.data(), ClientRemovePacket.size(),
			HIGH_PRIORITY, RELIABLE_ORDERED, 0,
			RakNet::UNASSIGNED_SYSTEM_ADDRESS,
			true, 0 );

		PeerInterface_->SendLoopback( ClientRemovePacket.data(), ClientRemovePacket.size() );
	}
}

#endif // !PLATFORM_HTML5 && !PLATFORM_WINPHONE
