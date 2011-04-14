/**************************************************************************
*
* File:		RmCore.cpp
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "RmCore.h"

SYS_CREATOR( RmCore );

////////////////////////////////////////////////////////////////////////////////
// Ctor
RmCore::RmCore()
{
	ConnectToggle_ = BcFalse;
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RmCore::~RmCore()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// register
void RmCore::registerDelegate( BcU32 UniqueID, RmDelegate Delegate )
{
	BcScopedLock< BcMutex > Lock( ContainerLock_ );
	BcAssert( DelegateMap_.find( UniqueID ) == DelegateMap_.end() );
	DelegateMap_[ UniqueID ] = Delegate;
}

////////////////////////////////////////////////////////////////////////////////
// unregister
void RmCore::unregisterDelegate( BcU32 UniqueID )
{
	BcScopedLock< BcMutex > Lock( ContainerLock_ );
	
	TDelegateMapIterator It = DelegateMap_.find( UniqueID );
	
	if( It != DelegateMap_.end() )
	{
		DelegateMap_.erase( It );
	}
}

////////////////////////////////////////////////////////////////////////////////
// send
BcBool RmCore::send( BcU32 UniqueID, void* pData, BcU32 Bytes )
{
	BcScopedLock< BcMutex > Lock( ContainerLock_ );

	BcBool Success = BcTrue;

	TMessageHeader Header( UniqueID, Bytes );
	TMessage Message( Header, new BcU8[ Bytes ] );
	
	BcMemCopy( Message.pData_, pData, Bytes );
	
	MessageList_.push_back( Message );

	return Success;
}

////////////////////////////////////////////////////////////////////////////////
// send
BcBool RmCore::send( BcU32 UniqueID, RmParameters& Params )
{
	return send( UniqueID, Params.getData(), Params.getDataSize() );
}


////////////////////////////////////////////////////////////////////////////////
// connect
void RmCore::connect( const BcChar* pAddress )
{
	BcScopedLock< BcMutex > Lock( ConnectionLock_ );

	// Listen for a connection by default.
	if( Connection_.connectRange( pAddress, 4000, 32 ) )
	{
		BcPrintf( "RmCore: Connected to %s on port %u\n", pAddress, Connection_.getPort() );
		ConnectToggle_ = BcTrue;
		EvtPublisher::publish( rmEVT_CONNECT_SUCCEEDED, RmEventCore() );
		EvtPublisher::publish( rmEVT_CONNECTION_CONNECTED, RmEventCore() );
	}
	else
	{
		BcPrintf( "RmCore: Failed to connect to %s.\n", pAddress );
		EvtPublisher::publish( rmEVT_CONNECT_FAILED, RmEventCore() );
	}
}

////////////////////////////////////////////////////////////////////////////////
// listen
void RmCore::listen()
{
	BcScopedLock< BcMutex > Lock( ConnectionLock_ );
	
	// Listen for a connection by default.
	if( Connection_.listenRange( "localhost", 4000, 32 ) )
	{
		BcPrintf( "RmCore: Incoming connection on port %u\n", Connection_.getPort() );
		ConnectToggle_ = BcTrue;
		EvtPublisher::publish( rmEVT_LISTEN_SUCCEEDED, RmEventCore() );
		EvtPublisher::publish( rmEVT_CONNECTION_CONNECTED, RmEventCore() );
	}
	else
	{
		BcPrintf( "RmCore: Failed to listen for connection.\n" );
		EvtPublisher::publish( rmEVT_LISTEN_FAILED, RmEventCore() );
	}
}

////////////////////////////////////////////////////////////////////////////////
// open
//virtual
void RmCore::open()
{
	BcScopedLock< BcMutex > Lock( ConnectionLock_ );

}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void RmCore::update()
{
	BcScopedLock< BcMutex > Lock( ConnectionLock_ );

	// Only process if we are connected.
	if( Connection_.isConnected() )
	{
		// Receive 1 message at a time.
		{
			TMessageHeader Header( 0, 0 );
			BcU32 BytesWaiting = Connection_.recv( &Header, sizeof( Header ), BcTrue, BcFalse );
						
			// If we have enough bytes waiting then we can read in the data.
			if( BytesWaiting == sizeof( Header ) )
			{
				// Pop the data off.
				Connection_.recv( &Header, sizeof( Header ), BcFalse, BcTrue );
		
				// Receive data.
				if( (BcU32)Header.Destination_ != BcErrorCode )
				{
					BcU8* pData = new BcU8[ Header.Bytes_ ];
					BcU32 DataSize = Connection_.recv( pData, Header.Bytes_, BcFalse, BcTrue );
		
					// Check the data.
					if( DataSize == (BcU32)Header.Bytes_ )
					{
						dispatch( Header, pData );
					}
					else
					{
						BcPrintf( "RmCore:: Error: Failed to receive %u bytes of data for delegate: 0x%x\n", (BcU32)Header.Bytes_, (BcU32)Header.Destination_ );
						Connection_.disconnect();			
					}
				}
			}
			/*
			else if( BytesWaiting == BcErrorCode )
			{
				Connection_.disconnect();
			}
			 */
		}
	
		// Send over 1 message at a time.
		{
			ContainerLock_.lock();
			if( MessageList_.size() > 0 )
			{
				TMessage Message = MessageList_.front();
				MessageList_.pop_front();
				ContainerLock_.unlock();
		
				BcBool Success = BcTrue;
			
				Success &= Connection_.send( &Message.Header_, sizeof( Message.Header_ ) );
		
				if( Success == BcTrue )
				{
					Success &= Connection_.send( Message.pData_, Message.Header_.Bytes_ );
			
					// Delete copied send data.
					BcU8* pData = (BcU8*)Message.pData_;
					delete [] pData;
				}
		
				// If sends have failed, disconnect.
				if( Success == BcFalse )
				{
					BcPrintf( "RmCore:: Error: Failed to send message of %u bytes to delegate 0x%x.\n", (BcU32)Message.Header_.Bytes_, (BcU32)Message.Header_.Destination_  );
					Connection_.disconnect();			
				}
			}
			else
			{
				ContainerLock_.unlock();			
			}
		}
		
		// Handle the keep alive message.
		keepAlive();
	}
	else
	{
		// Clear message list, can't communicate with anything.
		MessageList_.clear();

		// Publish disconnected event.
		if( ConnectToggle_ == BcTrue )
		{
			ConnectToggle_ = BcFalse;

			BcPrintf( "RmCore:: Disconnected.\n"  );
			EvtPublisher::publish( rmEVT_CONNECTION_DISCONNECTED, RmEventCore() );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// close
//virtual
void RmCore::close()
{
	BcScopedLock< BcMutex > Lock( ConnectionLock_ );

	// Disconnect.
	Connection_.disconnect();
}
										  
////////////////////////////////////////////////////////////////////////////////
// dispatch
void RmCore::dispatch( const TMessageHeader& Header, void* pData )
{
	RmDelegate Delegate;
	
	{
		BcScopedLock< BcMutex > Lock( ContainerLock_ );
		// Find the approriate delegate to send to.
		TDelegateMapIterator It = DelegateMap_.find( Header.Destination_ );
	
		if( It != DelegateMap_.end() )
		{
			Delegate = It->second;
		}
	}

	// Call if we have a delegate.
	if( Delegate.isValid() )
	{
		Delegate( pData, Header.Bytes_ );
	}
}

////////////////////////////////////////////////////////////////////////////////
// keepAlive
void RmCore::keepAlive()
{
	/*
	if( ++KeepAliveTicker_ > ( 60 * 5 ) )
	{
		KeepAliveTicker_ = 0;
		
		TMessageHeader MessageHeader( BcErrorCode, 0 );		
		if( Connection_.send( &MessageHeader, sizeof( MessageHeader ) ) == BcFalse )
		{
			Connection_.disconnect();
		}
	}
	*/
}


