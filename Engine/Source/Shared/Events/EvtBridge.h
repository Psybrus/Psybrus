/**************************************************************************
*
* File:		EvtBridge.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event bridge class.
*		
*		
*
* 
**************************************************************************/

#ifndef __EVTBRIDGE_H__
#define __EVTBRIDGE_H__

#include "EvtEvent.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations.
class EvtPublisher;

//////////////////////////////////////////////////////////////////////////
// EvtBridge
class EvtBridge
{
public:
	EvtBridge( EvtPublisher* pPublisher );
	virtual ~EvtBridge();

	/**
	 * Called when an event needs to be bridged.
	 */
	virtual void bridge( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize );

protected:
	/**
	 * Publish via the publisher.
	 */
	void publish( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize, BcBool AllowBridge = BcFalse, BcBool AllowProxy = BcFalse );


	EvtPublisher* pPublisher_;
	
	
};

#endif
