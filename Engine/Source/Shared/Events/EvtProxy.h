/**************************************************************************
*
* File:		EvtProxy.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event proxy class.
*		
*		
*
* 
**************************************************************************/

#ifndef __EVTPROXY_H__
#define __EVTPROXY_H__

#include "EvtEvent.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations.
class EvtPublisher;

//////////////////////////////////////////////////////////////////////////
// EvtProxy
class EvtProxy
{
public:
	EvtProxy( EvtPublisher* pPublisher );
	virtual ~EvtProxy();

	/**
	 * Called when an event needs to be proxied.
	 */
	virtual eEvtReturn proxy( EvtID ID, const EvtBaseEvent& EventBase );

protected:
	/**
	 * Publish via the publisher.
	 */
	void publish( EvtID ID, const EvtBaseEvent& EventBase, BcBool AllowProxy = BcFalse );

protected:
	EvtPublisher* pPublisher_;
	
	
};

#endif
