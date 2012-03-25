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
	evtEVT_PROXY_EVENT_BUFFER,
	evtEVT_PROXY_SYNC,	
};

struct EvtProxyEventBuffer: public EvtEvent< EvtProxyEventBuffer >
{
	BcU32 DataSize_;
	BcU32 ClientID_;
	BcU32 FrameIndex_;
};

struct EvtProxySyncEvent: public EvtEvent< EvtProxySyncEvent >
{
	BcU32 ClientID_;
	BcU32 FrameIndex_;
	BcU32 Checksum_;
};

////////////////////////////////////////////////////////////////////////////////
// EvtProxyLockstep
class EvtProxyLockstep: 
	public EvtProxy
{
public:
	EvtProxyLockstep( EvtPublisher* pPublisher, BcU32 ClientID, BcU32 NoofClients );
	virtual ~EvtProxyLockstep();

	void setFrameIndex( BcU32 Index, BcU32 Checksum );
	BcBool dispatchFrameIndex( BcU32 Index );
	BcBool isAhead() const;
	
private:
	virtual void proxy( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize );

private:
	eEvtReturn onProxyEventBuffer( EvtID ID, const EvtProxyEventBuffer& Event );
	eEvtReturn onProxySync( EvtID ID, const EvtProxySyncEvent& Event );

private:
	struct TEventBuffer
	{
		BcU32 FrameIndex_;
		BcStream* pEventStream_;
	};

	struct TClient
	{
		BcU32 FrameIndex_;
		BcU32 Checksum_;
	};

	typedef std::map< BcU32, TEventBuffer > TEventBufferMap;
	typedef TEventBufferMap::iterator TEventBufferMapIterator;

	BcU32 ClientID_;
	BcU32 NoofClients_;

	BcBool ClientAhead_;
	
	TEventBufferMap EventBufferMaps_[ 2 ];
	TClient Clients_[ 2 ];



	BcU32 CurrSchedulingFrameIndex_;
	
	BcU32 SyncEventsRecv_;
	BcU32 SyncEventsReq_;
	BcU32 SyncEventRate_;
	BcU32 SyncEventPendingIndex_;
	BcU32 SyncEventFrameIndex_;
};

#endif

