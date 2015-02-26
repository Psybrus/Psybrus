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
	pPublisher_->addProxy( this );
}

////////////////////////////////////////////////////////////////////////////////
// Ctor
//virtual
EvtProxy::~EvtProxy()
{
	pPublisher_->removeProxy( this );
}

////////////////////////////////////////////////////////////////////////////////
// proxy
//virtual
eEvtReturn EvtProxy::proxy( EvtID ID, const EvtBaseEvent& EventBase )
{
	// Pass event on to publisher.
	return evtRET_PASS;
}

////////////////////////////////////////////////////////////////////////////////
// publish
void EvtProxy::publish( EvtID ID, const EvtBaseEvent& EventBase, BcBool AllowProxy )
{
	pPublisher_->publishInternal( ID, EventBase, AllowProxy );
}
