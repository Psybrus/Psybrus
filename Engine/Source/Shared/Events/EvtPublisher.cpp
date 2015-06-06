/**************************************************************************
*
* File:		EvtPublisher.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event dispatcher.
*		
*		
*
* 
**************************************************************************/

#include "Events/EvtPublisher.h"

#include "Base/BcProfiler.h"
#include "Base/BcString.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
EvtPublisher::EvtPublisher()
{
	pParent_ = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
EvtPublisher::~EvtPublisher()
{
	BcAssertMsg( Proxies_.size() == 0, "EvtPublisher: Not all proxies have been removed." );	
}

////////////////////////////////////////////////////////////////////////////////
// publish
BcBool EvtPublisher::publish( EvtID ID, const EvtBaseEvent& EventBase, BcBool AllowProxy )
{
	return publishInternal( ID, EventBase, AllowProxy );
}

////////////////////////////////////////////////////////////////////////////////
// subscribe
void EvtPublisher::subscribe( EvtID ID, EvtBinding::BaseSignature Function )
{
	SubscribeList_.push_back( TBindingPair( ID, EvtBinding( (void*)Function, Function ) ) );
};

////////////////////////////////////////////////////////////////////////////////
// subscribe
void EvtPublisher::subscribe( EvtID ID, void* Owner, EvtBinding::BaseFunction Function )
{
	SubscribeList_.push_back( TBindingPair( ID, EvtBinding( Owner, Function ) ) );
};

////////////////////////////////////////////////////////////////////////////////
// unsubscribe
void EvtPublisher::unsubscribe( EvtID ID, EvtBinding::BaseSignature Function )
{
	UnsubscribeByOwnerList_.push_back( TOwnerPair( ID, (void*)Function ) );
}

////////////////////////////////////////////////////////////////////////////////
// unsubscribe
void EvtPublisher::unsubscribe( EvtID ID, void* pOwner )
{
	UnsubscribeByOwnerList_.push_back( TOwnerPair( ID, pOwner ) );
}

////////////////////////////////////////////////////////////////////////////////
// unsubscribeAll
void EvtPublisher::unsubscribeAll( void* pOwner )
{
	for( TBindingListMapIterator BindingListMapIterator = BindingListMap_.begin(); BindingListMapIterator != BindingListMap_.end(); ++BindingListMapIterator )
	{
		unsubscribeByOwner( BindingListMapIterator->first, pOwner );
	}
}

////////////////////////////////////////////////////////////////////////////////
// clearParent
void EvtPublisher::clearParent()
{
	pParent_ = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// setParent
void EvtPublisher::setParent( EvtPublisher* pParent )
{
	BcAssert( pParent_ == NULL );
	pParent_ = pParent;
}

////////////////////////////////////////////////////////////////////////////////
// removeProxy
void EvtPublisher::removeProxy( EvtProxy* pProxy )
{
	for( TProxyListIterator It( Proxies_.begin() ); It != Proxies_.end(); )
	{	
		if( (*It) == pProxy )
		{
			It = Proxies_.erase( It );
		}
		else
		{
			++It;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// addProxy
void EvtPublisher::addProxy( EvtProxy* pProxy )
{
	removeProxy( pProxy );
	Proxies_.push_back( pProxy );
}

////////////////////////////////////////////////////////////////////////////////
// publishInternal
BcBool EvtPublisher::publishInternal( EvtID ID, const EvtBaseEvent& EventBase, BcBool AllowProxy )
{
	BcAssert( BcIsGameThread() );

	PSY_PROFILER_INSTANT_EVENT( "EvtPublisher::publishInternal" );

	// Proxy event through all attached proxies if this event allows it.
	if( AllowProxy == BcTrue )
	{
		for( TProxyListIterator It( Proxies_.begin() ); It != Proxies_.end(); ++It )
		{	
			EvtProxy* pProxy( *It );
			eEvtReturn RetVal = pProxy->proxy( ID, EventBase );

			switch( RetVal )
			{
			// Event passed. Publisher, or next proxy can deal with it.
			case evtRET_PASS:
				break;
			// Event blocked. If we are a parent, we want our child publisher to abort (normal behaviour).
			case evtRET_BLOCK:
				return BcFalse;
				break;
			// Unsupported enum value.
			default:
				BcBreakpoint;
				break;
			}
		}
	}

	// Update binding map before going ahead.
	updateBindingMap();
	
	// If we have a parent, publish to them first.
	BcBool ShouldPublish = BcTrue;
	
	if( pParent_ != NULL )
	{
		ShouldPublish = pParent_->publishInternal( ID, EventBase, AllowProxy );
	}

	// Only publish if the previous call to our parent allows us to.
	if( ShouldPublish == BcTrue )
	{
		// Find the appropriate binding list.
		TBindingListMapIterator BindingListMapIterator = BindingListMap_.find( ID );
		
		// Add list if we need to, and grab iterator.
		if( BindingListMapIterator != BindingListMap_.end() )
		{
			// Iterate over all bindings in list and call.
			TBindingList& BindingList = BindingListMapIterator->second;
			TBindingListIterator Iter = BindingList.begin();
			
			while( Iter != BindingList.end() )
			{
				EvtBinding& Binding = (*Iter);
				
				// Call binding and handle it's return.
				eEvtReturn RetVal = Binding( ID, EventBase );
				switch( RetVal )
				{
					case evtRET_PASS:
						++Iter;
						break;

					case evtRET_BLOCK:
						return BcFalse;
						break;

					case evtRET_REMOVE:
						Iter = BindingList.erase( Iter );
						break;
						
					default:
						BcBreakpoint;
						break;
				}
			}
		}
	}

	return BcTrue;
}

////////////////////////////////////////////////////////////////////////////////
// subscribeInternal
void EvtPublisher::subscribeInternal( EvtID ID, const EvtBinding& Binding )
{
	// Find the appropriate binding list.
	TBindingListMapIterator BindingListMapIterator = BindingListMap_.find( ID );
	
	// Add list if we need to, and grab iterator.
	if( BindingListMapIterator == BindingListMap_.end() )
	{
		BindingListMap_[ ID ] = TBindingList();
		BindingListMapIterator = BindingListMap_.find( ID );
	}
	
	// Append binding to list.
	TBindingList& BindingList = BindingListMapIterator->second;
	BindingList.push_back( Binding );
}

////////////////////////////////////////////////////////////////////////////////
// unsubscribeByOwner
void EvtPublisher::unsubscribeByOwner( EvtID ID, void* pOwner )
{
	// Find the appropriate binding list.
	TBindingListMapIterator BindingListMapIterator = BindingListMap_.find( ID );
	
	// Add list if we need to, and grab iterator.
	if( BindingListMapIterator != BindingListMap_.end() )
	{
		// Find the matching binding.
		TBindingList& BindingList = BindingListMapIterator->second;
		TBindingListIterator Iter = BindingList.begin();
		
		while( Iter != BindingList.end() )
		{
			if( (*Iter).getOwner() == pOwner )
			{
				Iter = BindingList.erase( Iter );
			}
			else
			{
				++Iter;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// updateBindingMap
void EvtPublisher::updateBindingMap()
{
	// Subscribe.
	for( TBindingPairListIterator Iter = SubscribeList_.begin(); Iter != SubscribeList_.end(); Iter = SubscribeList_.erase( Iter ) )
	{
		subscribeInternal( Iter->first, Iter->second );
	}

	// Unsubscribe by owner.
	for( TOwnerPairListIterator Iter = UnsubscribeByOwnerList_.begin(); Iter != UnsubscribeByOwnerList_.end(); Iter = UnsubscribeByOwnerList_.erase( Iter ) )
	{
		unsubscribeByOwner( Iter->first, Iter->second );
	}
}
