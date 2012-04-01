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
	virtual void proxy( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize );

protected:
	/**
	 * Publish via the publisher.
	 */
	void publish( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize, BcBool AllowBridge = BcFalse, BcBool AllowProxy = BcFalse );

protected:
	EvtPublisher* pPublisher_;
	
	
};

#endif
