/**************************************************************************
*
* File:		EvtProxyBuffered.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event proxy buffered. Buffers events for controlled dispatch.
*		
*		
*
* 
**************************************************************************/

#ifndef __EVTPROXYBUFFERED_H__
#define __EVTPROXYBUFFERED_H__

#include "EvtProxy.h"

//////////////////////////////////////////////////////////////////////////
// EvtProxyBuffered
class EvtProxyBuffered:
	public EvtProxy
{
public:
	EvtProxyBuffered( EvtPublisher* pPublisher, BcSize BufferSize = ( 1 * 1024 ) );
	virtual ~EvtProxyBuffered();

	/**
	 * Called when an event needs to be proxied.
	 */
	virtual void proxy( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize );

	/**
	 * Dispatch all the events.
	 */
	void dispatch();

protected:
	struct TEventPackage
	{
		EvtID ID_;
		BcSize Size_;
		void* pEventData_;
	};

	typedef std::vector< TEventPackage > TEventPackageList;
	TEventPackageList Events_;
};

#endif // __EVTPROXYBUFFERED_H__