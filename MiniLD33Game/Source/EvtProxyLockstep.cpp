/**************************************************************************
*
* File:		EvtProxyLockstep.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event proxy for lockstep.
*		
*		
*
* 
**************************************************************************/

#include "EvtProxyLockstep.h"

////////////////////////////////////////////////////////////////////////////////
// EvtProxyLockstep
EvtProxyLockstep::EvtProxyLockstep( EvtPublisher* pPublisher, BcU32 ClientID, BcU32 NoofClients ):
	EvtProxy( pPublisher )
{
	ClientID_ = ClientID;
	NoofClients_ = NoofClients;

	BcAssert( NoofClients_ <= 2 );

	CurrSchedulingFrameIndex_ = 0;
	SyncEventsRecv_ = 0;
	SyncEventsReq_ = 1;
	SyncEventRate_ = 4;
	SyncEventPendingIndex_ = 0;
	SyncEventFrameIndex_ = SyncEventRate_ * 2;

	BcMemZero( &Clients_, sizeof( Clients_ ) );
	
	EvtProxyEventBuffer::Delegate OnProxyEventBuffer( EvtProxyEventBuffer::Delegate::bind< EvtProxyLockstep, &EvtProxyLockstep::onProxyEventBuffer >( this ) );
	EvtProxySyncEvent::Delegate OnProxySync( EvtProxySyncEvent::Delegate::bind< EvtProxyLockstep, &EvtProxyLockstep::onProxySync >( this ) );

	pPublisher_->subscribe( evtEVT_PROXY_EVENT_BUFFER, OnProxyEventBuffer );
	pPublisher_->subscribe( evtEVT_PROXY_SYNC, OnProxySync );
}

////////////////////////////////////////////////////////////////////////////////
// EvtProxyLockstep
//virtual
EvtProxyLockstep::~EvtProxyLockstep()
{
	pPublisher_->unsubscribeAll( this );
}

////////////////////////////////////////////////////////////////////////////////
// setFrameIndex
void EvtProxyLockstep::setFrameIndex( BcU32 Index )
{
	// Use sync rate to determine scheduling.
	Index += SyncEventRate_ * 2;

	// If we're using a different scheduling frame index, continue on.
	if( Index != CurrSchedulingFrameIndex_ )
	{
		TEventBufferMap& EventBufferMap = EventBufferMaps_[ ClientID_ ];

		// Send previous event buffer.
		TEventBufferMapIterator ItPrev( EventBufferMap.find( CurrSchedulingFrameIndex_ ) );
		
		if( ItPrev != EventBufferMap.end() )
		{
			TEventBuffer& EventBuffer = (*ItPrev).second;
			BcAssert( EventBuffer.FrameIndex_ == CurrSchedulingFrameIndex_ );
			
			BcU8* pData = EventBuffer.pEventStream_->pData();
			BcU32 DataSize = EventBuffer.pEventStream_->dataSize();

			// Don't send empty event buffers.
			if( DataSize > sizeof( EvtProxyEventBuffer ) )
			{
				EvtProxyEventBuffer& Event( *reinterpret_cast< EvtProxyEventBuffer* >( pData ) );
				Event.DataSize_ = DataSize; // Push data size in.
				publish( evtEVT_PROXY_EVENT_BUFFER, Event, DataSize, BcTrue, BcFalse );
			}
		}

		// Send where we are allowed to simulate until (1 less than current).
		if( ( Index % SyncEventRate_ ) == 0 )
		{
			EvtProxySyncEvent Event;
			Event.ClientID_ = ClientID_;
			Event.FrameIndex_ = Index - 1;
			publish( evtEVT_PROXY_SYNC, Event, sizeof( Event ), BcTrue, BcFalse );
		}
		
		// Add new.
		TEventBufferMapIterator ItCurr( EventBufferMap.find( Index ) );
		CurrSchedulingFrameIndex_ = Index;

		if( ItCurr == EventBufferMap.end() )
		{
			TEventBuffer EventBuffer =
			{
				CurrSchedulingFrameIndex_,
				new BcStream( BcFalse, 32, 32 )
			};

			EvtProxyEventBuffer Event;

			Event.ClientID_ = ClientID_;
			Event.FrameIndex_ = EventBuffer.FrameIndex_;
			Event.DataSize_ = sizeof( 0 );
			
			// Add frame index to buffer.
			EventBuffer.pEventStream_->push( &Event, sizeof( Event ) );
			
			// Add to map.
			EventBufferMap[ Index ] = EventBuffer;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// dispatchFrameIndex
BcBool EvtProxyLockstep::dispatchFrameIndex( BcU32 Index )
{
	// Determine if all clients are up to the right sync./
	BcU32 MaxSyncIndex = BcErrorCode;
	for( BcU32 ClientIdx = 0; ClientIdx < NoofClients_; ++ClientIdx )
	{
		TClient& Client = Clients_[ ClientIdx ];
		MaxSyncIndex = BcMin( MaxSyncIndex, Client.FrameIndex_ );
	}
	
	// If the max sync index is less or equal to the one we are trying to execute we can dispatch events.
	if( Index <= MaxSyncIndex )
	{
		for( BcU32 ClientIdx = 0; ClientIdx < NoofClients_; ++ClientIdx )
		{
			TEventBufferMap& EventBufferMap = EventBufferMaps_[ ClientIdx ];
			TEventBufferMapIterator It( EventBufferMap.find( Index ) );

			if( It != EventBufferMap.end() )
			{
				TEventBuffer& EventBuffer = (*It).second;
				BcAssert( EventBuffer.FrameIndex_ == Index );

				BcU8* pData = EventBuffer.pEventStream_->pData();
				BcU32 DataSize = EventBuffer.pEventStream_->dataSize();
				BcU32 ReadPtr = 0;

				// Grab the header.
				EvtProxyEventBuffer& Event = *reinterpret_cast< EvtProxyEventBuffer* >( &pData[ ReadPtr ] ); ReadPtr += sizeof( EvtProxyEventBuffer );
				BcAssert( Event.FrameIndex_ == EventBuffer.FrameIndex_ );
				BcAssert( Event.ClientID_ == ClientIdx );

				if( ReadPtr < DataSize )
				{
					BcPrintf( "LockstepDispatch(%u): Bytes: %u\n", Index, DataSize );

					while( ReadPtr < DataSize )
					{
						BcU32 ID = *reinterpret_cast< BcU32* >( &pData[ ReadPtr ] ); ReadPtr += sizeof( BcU32 );
						BcU32 EventSize = *reinterpret_cast< BcU32* >( &pData[ ReadPtr ] ); ReadPtr += sizeof( BcU32 );
						const EvtBaseEvent& EventBase = *reinterpret_cast< EvtBaseEvent* >( &pData[ ReadPtr ] ); ReadPtr += EventSize;

						//BcPrintf( "D(%u): %u\n", Index, ID );

						// Publish.
						publish( ID, EventBase, EventSize );
					}
				}
				// Clean up and erase event buffer.
				delete EventBuffer.pEventStream_;
				EventBufferMap.erase( It );
			}
		}

		return BcTrue;
	}
	else
	{
		return BcFalse;
	}
}

////////////////////////////////////////////////////////////////////////////////
// proxy
//virtual
void EvtProxyLockstep::proxy( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize )
{
	TEventBufferMap& EventBufferMap = EventBufferMaps_[ ClientID_ ];
	TEventBufferMapIterator It( EventBufferMap.find( CurrSchedulingFrameIndex_ ) );

	if( It != EventBufferMap.end() )
	{
		TEventBuffer& EventBuffer = (*It).second;
		BcAssert( EventBuffer.FrameIndex_ == CurrSchedulingFrameIndex_ );

		// Encode event into stream.
		BcU32 SendID = ID;
		BcU32 SendEventSize = EventSize;
		EventBuffer.pEventStream_->push( &SendID, sizeof( SendID ) );
		EventBuffer.pEventStream_->push( &SendEventSize, sizeof( SendEventSize ) );
		EventBuffer.pEventStream_->push( &EventBase, EventSize );
	}
}

////////////////////////////////////////////////////////////////////////////////
// proxy
eEvtReturn EvtProxyLockstep::onProxyEventBuffer( EvtID ID, const EvtProxyEventBuffer& Event )
{
	const BcU8* pData = reinterpret_cast< const BcU8* >( &Event ) + sizeof( EvtProxyEventBuffer );
	BcU32 DataSize = Event.DataSize_;
		

	
	return evtRET_PASS;
}

////////////////////////////////////////////////////////////////////////////////
// proxy
eEvtReturn EvtProxyLockstep::onProxySync( EvtID ID, const EvtProxySyncEvent& Event )
{
	BcAssert( Event.ClientID_ < NoofClients_ );
	TClient& Client = Clients_[ Event.ClientID_ ];
	BcAssert( Client.FrameIndex_ <= Event.FrameIndex_ ); // First sync will always be equal.
	Client.FrameIndex_ = Event.FrameIndex_;

	return evtRET_PASS;
}
