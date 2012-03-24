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
EvtProxyLockstep::EvtProxyLockstep( EvtPublisher* pPublisher ):
	EvtProxy( pPublisher )
{
	CurrFrameIndex_ = BcErrorCode;	
}

////////////////////////////////////////////////////////////////////////////////
// EvtProxyLockstep
//virtual
EvtProxyLockstep::~EvtProxyLockstep()
{

}

////////////////////////////////////////////////////////////////////////////////
// setFrameIndex
void EvtProxyLockstep::setFrameIndex( BcU32 Index )
{
	if( Index != CurrFrameIndex_ )
	{
		// Send previous event buffer.
		TEventBufferMapIterator ItPrev( EventBufferMap_.find( CurrFrameIndex_ ) );
		
		if( ItPrev != EventBufferMap_.end() )
		{
			TEventBuffer& EventBuffer = (*ItPrev).second;
			BcAssert( EventBuffer.FrameIndex_ == CurrFrameIndex_ );
			
			BcU8* pData = EventBuffer.pEventStream_->pData();
			BcU32 DataSize = EventBuffer.pEventStream_->dataSize();

			// Don't send individual frames.
			if( DataSize > 4 )
			{
				const EvtBaseEvent& BaseEvent( *reinterpret_cast< EvtBaseEvent* >( pData ) );
				publish( evtEVT_PROXY_EVENT_BUFFER, BaseEvent, DataSize, BcTrue, BcFalse );
			}
		}
		
		// Add new.
		TEventBufferMapIterator ItCurr( EventBufferMap_.find( Index ) );
		CurrFrameIndex_ = Index;

		if( ItCurr == EventBufferMap_.end() )
		{
			TEventBuffer EventBuffer =
			{
				CurrFrameIndex_,
				new BcStream( BcFalse, 32, 32 )
			};
	
			// Add frame index to buffer.
			EventBuffer.pEventStream_->push( &EventBuffer.FrameIndex_, sizeof( BcU32 ) );
	
			// Add to map.
			EventBufferMap_[ Index ] = EventBuffer;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// dispatchFrameIndex
void EvtProxyLockstep::dispatchFrameIndex( BcU32 Index )
{
	TEventBufferMapIterator It( EventBufferMap_.find( Index ) );

	if( It != EventBufferMap_.end() )
	{
		TEventBuffer& EventBuffer = (*It).second;
		BcAssert( EventBuffer.FrameIndex_ == Index );

		BcU8* pData = EventBuffer.pEventStream_->pData();
		BcU32 DataSize = EventBuffer.pEventStream_->dataSize();
		BcU32 ReadPtr = 0;
		
		BcU32 FrameIndex = *reinterpret_cast< BcU32* >( &pData[ ReadPtr ] ); ReadPtr += sizeof( BcU32 );
		BcAssert( EventBuffer.FrameIndex_ == FrameIndex );

		if( ReadPtr < DataSize )
		{
			BcPrintf( "LockstepDispatch(%u): Bytes: %u\n", Index, DataSize );

			while( ReadPtr < DataSize )
			{
				// Decode event.
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
		EventBufferMap_.erase( It );
	}
}

////////////////////////////////////////////////////////////////////////////////
// proxy
//virtual
void EvtProxyLockstep::proxy( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize )
{
	TEventBufferMapIterator It( EventBufferMap_.find( CurrFrameIndex_ ) );

	if( It != EventBufferMap_.end() )
	{
		TEventBuffer& EventBuffer = (*It).second;
		BcAssert( EventBuffer.FrameIndex_ == CurrFrameIndex_ );

		// Encode event into stream.
		BcU32 SendID = ID;
		BcU32 SendEventSize = EventSize;
		EventBuffer.pEventStream_->push( &SendID, sizeof( SendID ) );
		EventBuffer.pEventStream_->push( &SendEventSize, sizeof( SendEventSize ) );
		EventBuffer.pEventStream_->push( &EventBase, EventSize );

		//BcPrintf( "P(%u): %u\n", CurrFrameIndex_, ID );
	}
}
