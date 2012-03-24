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
EvtProxy::EvtProxy( EvtPublisher* pPublisher ):
	pPublisher_( pPublisher )
{
	pPublisher_->setProxy( this );
}

////////////////////////////////////////////////////////////////////////////////
// Ctor
//virtual
EvtProxy::~EvtProxy()
{
	pPublisher_->clearProxy();
}

////////////////////////////////////////////////////////////////////////////////
// proxy
//virtual
void EvtProxy::proxy( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize )
{
	publish( ID, EventBase, EventSize );
}

////////////////////////////////////////////////////////////////////////////////
// publish
void EvtProxy::publish( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize, BcBool AllowBridge, BcBool AllowProxy )
{
	pPublisher_->publishInternal( ID, EventBase, EventSize, AllowBridge, AllowProxy );
}
