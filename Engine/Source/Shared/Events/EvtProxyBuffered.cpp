/**************************************************************************
*
* File:		EvtProxyBuffered.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event proxy buffered. Buffers events for controlled dispatch.
*		
*		
*
* 
**************************************************************************/

#include "EvtProxyBuffered.h"

#include "Base/BcMemory.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
EvtProxyBuffered::EvtProxyBuffered( EvtPublisher* pPublisher, BcSize BufferSize ):
	EvtProxy( pPublisher )
{
	//BufferSize_ = BufferSize;
	//BufferPosition_ = 0;
	//pBuffer_ = BcMemAlign( BufferSize, 16 );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
EvtProxyBuffered::~EvtProxyBuffered()
{
	//BcMemFree( pBuffer_ );
	//pBuffer_ = NULL;
	//BufferSize_ = 0;
	//BufferPosition_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// proxy
//virtual
void EvtProxyBuffered::proxy( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize )
{
	// Create a management object for event.
	TEventPackage Event;
	Event.ID_ = ID;
	Event.Size_ = EventSize;
	Event.pEventData_ = BcMemAlign( EventSize, 16 ); 
	BcMemCopy( Event.pEventData_, &EventBase, EventSize );

	// Push into vector.
	Events_.push_back( Event );
}

//////////////////////////////////////////////////////////////////////////
// dispatch
void EvtProxyBuffered::dispatch()
{
	if( Events_.size() > 0 )
	{
		// Cache events and clear original.
		TEventPackageList Events( Events_ );
		Events_.clear();

		for( BcU32 Idx = 0; Idx < Events.size(); ++Idx )
		{
			TEventPackage& Event( Events[ Idx ] );
			EvtProxy::publish( Event.ID_, *reinterpret_cast< EvtBaseEvent* >( Event.pEventData_ ), Event.Size_ );
			BcMemFree( Event.pEventData_ );
		}
	}
}
