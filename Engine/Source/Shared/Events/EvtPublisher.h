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

#include "Events/EvtEvent.h"
#include "Events/EvtBinding.h"
#include "Events/EvtProxy.h"

#include <list>
#include <map>


////////////////////////////////////////////////////////////////////////////////
/**	\class EvtPublisher
*	\brief Event publisher class.
*
*	Any class wishing to publish events should derive from this base.<br/>
*	All events should be published from the same thread.
*	It is not thread safe for subscription/un-subscription, or publication.
*	All operations should be performed on the main thread.
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
	void publish( EvtID ID, const EvtEvent< _Ty >& Event, BcBool AllowProxy = BcTrue );

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

	/**
	 * Clear parent.
	 */
	void clearParent();

	/**
	 * Set parent.
	 */
	void setParent( EvtPublisher* pParent );

	/**
	 * Remove proxy.
	 */
	void removeProxy( EvtProxy* pProxy );

	/**
	 * Add proxy.
	 */
	void addProxy( EvtProxy* pProxy );

private:
	/**
	* Publish internal.
	*/
	BcBool publishInternal( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize, BcBool AllowProxy = BcTrue );
	
	/**
	* Subscribe internal.
	*/
	void subscribeInternal( EvtID ID, const EvtBinding& Binding );
	
	/**
	* Unsubscribe internal.
	*/
	void unsubscribeInternal( EvtID ID, const EvtBinding& Binding );

	/**
	* Unsubscribe by owner.
	*/
	void unsubscribeByOwner( EvtID ID, void* );

	/**
	* Update binding map.
	*/
	void updateBindingMap();

private:
	friend class EvtProxy;
	friend class EvtBridge;

	typedef std::vector< EvtBinding >			TBindingList;
	typedef TBindingList::iterator				TBindingListIterator;
	typedef std::map< EvtID, TBindingList >		TBindingListMap;
	typedef TBindingListMap::iterator			TBindingListMapIterator;	
	typedef std::pair< EvtID, EvtBinding >		TBindingPair;
	typedef std::vector< TBindingPair >			TBindingPairList;
	typedef TBindingPairList::iterator			TBindingPairListIterator;
	typedef std::pair< EvtID, void* >			TOwnerPair;
	typedef std::vector< TOwnerPair >			TOwnerPairList;
	typedef TOwnerPairList::iterator			TOwnerPairListIterator;
	typedef std::list< EvtProxy* >				TProxyList;
	typedef TProxyList::iterator				TProxyListIterator;

	TBindingListMap								BindingListMap_;			///!< Bind list map.
	TBindingPairList							SubscribeList_;				///!< List of bindings to add to the map.
	TBindingPairList							UnsubscribeList_;			///!< List of bindings to remove from the map.
	TOwnerPairList								UnsubscribeByOwnerList_;	///!< List of owners to remove from the map.
	TProxyList									Proxies_;					///!< Proxies we have.
	EvtPublisher*								pParent_;					///!< Parent publisher.
};

////////////////////////////////////////////////////////////////////////////////
// Inlines
template< typename _Ty >
BcForceInline void EvtPublisher::publish( EvtID ID, const EvtEvent< _Ty >& Event, BcBool AllowProxy )
{
	publishInternal( ID, Event, sizeof( _Ty ), AllowProxy );
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
