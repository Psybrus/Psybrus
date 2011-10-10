/**************************************************************************
*
* File:		EvtPublisher.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event dispatcher.
*		
*		
*
* 
**************************************************************************/

#ifndef __EVTPUBLISHER_H__
#define __EVTPUBLISHER_H__

#include "EvtEvent.h"
#include "EvtBinding.h"

#include <list>
#include <map>

////////////////////////////////////////////////////////////////////////////////
/**	\class EvtPublisher
*	\brief Event publisher class.
*
*	Any class wishing to publish events should derive from this base.<br/>
*	All events should be published from the same thread.
*	It is not currently thread safe for subscription/un-subscription, but
*	it will be in the near future.
*/
class EvtPublisher
{
public:
	EvtPublisher();
	virtual ~EvtPublisher();
	
	/**
	* Publish an event.
	*/
	template< typename _Ty >
	void publish( EvtID ID, const EvtEvent< _Ty >& Event );

	/**
	* Subscribe to an event.
	*/
	template< typename _Ty >
	void subscribe( EvtID ID, BcDelegate< eEvtReturn(*)( EvtID, const _Ty& ) > ExternalDelegate );
	
	/**
	* Unsubscribe from an event.
	*/
	template< typename _Ty >
	void unsubscribe( EvtID ID, BcDelegate< eEvtReturn(*)( EvtID, const _Ty& ) > ExternalDelegate );
	
	/**
	 * Unsubscribe from an event using owner pointer.
	 */
	void unsubscribe( EvtID ID, void* pOwner );

	/**
	 * Unsubscribe from all events using owner pointer.
	 */
	void unsubscribeAll( void* pOwner );

private:
	/**
	* Publish internal.
	*/
	void publishInternal( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize );
	
	/**
	* Subscribe internal.
	*/
	void subscribeInternal( EvtID ID, const EvtBinding& Binding );
	
	/**
	* Unsubscribe internal.
	*/
	void unsubscribeInternal( EvtID ID, const EvtBinding& Binding );

	/**
	* Update binding map.
	*/
	void updateBindingMap();

private:
	typedef std::vector< EvtBinding >			TBindingList;
	typedef TBindingList::iterator				TBindingListIterator;
	typedef std::map< EvtID, TBindingList >		TBindingListMap;
	typedef TBindingListMap::iterator			TBindingListMapIterator;	
	typedef std::pair< EvtID, EvtBinding >		TBindingPair;
	typedef std::vector< TBindingPair >			TBindingPairList;
	typedef TBindingPairList::iterator			TBindingPairListIterator;

	TBindingListMap								BindingListMap_;			///!< Bind list map.
	TBindingPairList							SubscribeList_;				///!< List of bindings to add to the map.
	TBindingPairList							UnsubscribeList_;			///!< List of bindings to remove from the map.
};

////////////////////////////////////////////////////////////////////////////////
// Inlines
template< typename _Ty >
BcForceInline void EvtPublisher::publish( EvtID ID, const EvtEvent< _Ty >& Event )
{
	publishInternal( ID, Event, sizeof( _Ty ) );
}

template< typename _Ty >
BcForceInline void EvtPublisher::subscribe( EvtID ID, BcDelegate< eEvtReturn(*)( EvtID, const _Ty& ) > ExternalDelegate )
{
	SubscribeList_.push_back( TBindingPair( ID, EvtBinding( ExternalDelegate ) ) );
};

template< typename _Ty >
BcForceInline void EvtPublisher::unsubscribe( EvtID ID, BcDelegate< eEvtReturn(*)( EvtID, const _Ty& ) > ExternalDelegate )
{
	UnsubscribeList_.push_back( TBindingPair( ID, EvtBinding( ExternalDelegate ) ) );
};

#endif
