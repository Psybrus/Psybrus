/**************************************************************************
*
* File:		EvtProxyLockstep.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event proxy for lockstep.
*		
*		
*
* 
**************************************************************************/

#ifndef __EVTPROXYLOCKSTEP_H__
#define __EVTPROXYLOCKSTEP_H__

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// Events
#define EVT_EVENTGROUP_PROXY	EVT_MAKE_ID( 'E', 'v', 0xff )

enum EvtProxyEvents
{
	evtEVT_PROXY_FIRST = EVT_EVENTGROUP_PROXY,
	evtEVT_PROXY_EVENT_BUFFER
};

////////////////////////////////////////////////////////////////////////////////
// EvtProxyLockstep
class EvtProxyLockstep: 
	public EvtProxy
{
public:
	EvtProxyLockstep( EvtPublisher* pPublisher );
	virtual ~EvtProxyLockstep();

	void setFrameIndex( BcU32 Index );
	void dispatchFrameIndex( BcU32 Index );
	
private:
	virtual void proxy( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize );

private:
	struct TEventBuffer
	{
		BcU32 FrameIndex_;
		BcStream* pEventStream_;
	};

	typedef std::map< BcU32, TEventBuffer > TEventBufferMap;
	typedef TEventBufferMap::iterator TEventBufferMapIterator;
	
	BcU32 CurrFrameIndex_;
	TEventBufferMap EventBufferMap_;
	
};

#endif

