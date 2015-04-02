#include "System/Network/NsEventProxy.h"
#include "System/Network/NsSession.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
NsEventProxy::NsEventProxy( class EvtPublisher* Publisher, NsSession* Session, NsEventProxyID ProxyID ):
	EvtProxy( Publisher ),
	Session_( Session ),
	ProxyID_( ProxyID )
{
	BcAssert( Session_ != nullptr );
	Session_->registerMessageHandler( ProxyID_, this );
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
NsEventProxy::~NsEventProxy()
{
	Session_->deregisterMessageHandler( ProxyID_, this );
}

////////////////////////////////////////////////////////////////////////////////
// NsEventProxy
eEvtReturn NsEventProxy::proxy( EvtID EventID, const EvtBaseEvent& EventBase )
{
	BcAssert( BcIsGameThread() );

	Payload_.Header_.ProxyID_ = ProxyID_;
	Payload_.Header_.EventID_ = EventID;
	BcMemCopy( Payload_.Data_, &EventBase, EventBase.size() ); 
	BcU32 PayloadSize = EventBase.size() + sizeof( Header );

	Session_->broadcast( ProxyID_, reinterpret_cast< const BcU8* >( &Payload_ ), PayloadSize,
		NsPriority::HIGH, NsReliability::RELIABLE_ORDERED );

	return evtRET_BLOCK;
}

////////////////////////////////////////////////////////////////////////////////
// onMessageReceived
void NsEventProxy::onMessageReceived( const void* Data, size_t DataSize )
{
	BcAssert( BcIsGameThread() );
	BcAssert( DataSize >= sizeof( Header ) );
	BcAssert( DataSize <= sizeof( Payload ) );
	const Payload* InPayload = reinterpret_cast< const Payload* >( Data );
	BcAssertMsg( InPayload->Header_.ProxyID_ == ProxyID_, "Received data from another proxy. IDs should match." );
	publish( InPayload->Header_.EventID_, *reinterpret_cast< const EvtBaseEvent* >( InPayload->Data_ ) );
}

