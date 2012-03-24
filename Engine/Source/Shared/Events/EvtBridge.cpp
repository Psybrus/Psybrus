/**************************************************************************
*
* File:		EvtProxy.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event proxy class.
*		
*		
*
* 
**************************************************************************/

#include "Events/EvtProxy.h"
#include "Events/EvtPublisher.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
EvtBridge::EvtBridge( EvtPublisher* pPublisher ):
	pPublisher_( pPublisher )
{
	pPublisher_->setBridge( this );
}

////////////////////////////////////////////////////////////////////////////////
// Ctor
//virtual
EvtBridge::~EvtBridge()
{
	pPublisher_->clearBridge();
}

////////////////////////////////////////////////////////////////////////////////
// proxy
//virtual
void EvtBridge::bridge( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize )
{
	BcUnusedVar( ID );
	BcUnusedVar( EventBase );
	BcUnusedVar( EventSize );
}

////////////////////////////////////////////////////////////////////////////////
// publish
void EvtBridge::publish( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize, BcBool AllowBridge, BcBool AllowProxy )
{
	pPublisher_->publishInternal( ID, EventBase, EventSize, AllowBridge, AllowProxy );
}
