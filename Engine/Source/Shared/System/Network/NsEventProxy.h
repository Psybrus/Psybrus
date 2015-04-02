#pragma once

#include "Events/EvtProxy.h"
#include "System/Network/NsTypes.h"
#include "System/Network/NsSession.h"


////////////////////////////////////////////////////////////////////////////////
// NsEventProxy
class NsEventProxy:
	public EvtProxy,
	public NsSessionMessageHandler
{
public:
	NsEventProxy( class EvtPublisher* Publisher, class NsSession* Session, NsEventProxyID ProxyID );
	virtual ~NsEventProxy();

	eEvtReturn proxy( EvtID EventID, const EvtBaseEvent& EventBase ) override;

	void onMessageReceived( const void* Data, size_t DataSize ) override;

private:
	NsEventProxyID ProxyID_;
	class NsSession* Session_;

	struct Header
	{
		NsEventProxyID ProxyID_;
		EvtID EventID_;
	};

	struct Payload
	{
		Header Header_;
		BcU8 Data_[ EvtBaseEvent::MAX_EVENT_SIZE ];
	};

	Payload Payload_;
};
