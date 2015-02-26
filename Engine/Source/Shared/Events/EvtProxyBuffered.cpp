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
eEvtReturn EvtProxyBuffered::proxy( EvtID ID, const EvtBaseEvent& EventBase )
{
	// Create a management object for event.
	TEventPackage EventPackage;
	EventPackage.ID_ = ID;
	// TODO: Erm, switch over to a linear allocator for events.
	//       Perhaps share across *all* EvtProxyBuffered objects.
	EventPackage.pEventData_ = BcMemAlign( EventBase.size(), 16 ); 
	BcMemCopy( EventPackage.pEventData_, &EventBase, EventBase.size() );

	// Push into vector.
	Events_.push_back( EventPackage );

	// Always block, we will dispatch it.
	return evtRET_BLOCK;
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
			TEventPackage& EventPackage( Events[ Idx ] );
			const auto& EventBase = *reinterpret_cast< EvtBaseEvent* >( EventPackage.pEventData_ );
			EvtProxy::publish( 
				EventPackage.ID_, 
				EventBase );
			BcMemFree( EventPackage.pEventData_ );
		}
	}
}
