/**************************************************************************
*
* File:		EvtProxyDebug.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event proxy debugger. Logs all events which pass through.
*		
*		
*
* 
**************************************************************************/

#ifndef __EvtProxyDebug_H__
#define __EvtProxyDebug_H__

#include "EvtProxy.h"

#include "Base/BcName.h"

//////////////////////////////////////////////////////////////////////////
// EvtProxyDebug
class EvtProxyDebug:
	public EvtProxy
{
public:
	EvtProxyDebug( EvtPublisher* pPublisher, const BcName& Tag );
	virtual ~EvtProxyDebug();

	/**
	 * Called when an event needs to be proxied.
	 */
	virtual eEvtReturn proxy( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize );

private:
	BcName Tag_;
};

#endif // __EvtProxyDebug_H__