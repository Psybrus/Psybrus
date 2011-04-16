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

#include "EvtPublisher.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
EvtPublisher::EvtPublisher()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
EvtPublisher::~EvtPublisher()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// publishInternal
void EvtPublisher::publishInternal( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize )
{
	BcUnusedVar( EventSize );
	
	/*
	{
		BcChar PrefixA = ( ID >> 24 ) & 0xff;
		BcChar PrefixB = ( ID >> 16 ) & 0xff;
		BcU32 Group = ( ID >> 8 ) & 0xff;
		BcU32 Item = ( ID ) & 0xff;
		
		BcPrintf( "EvtPublish: %x, \"%c%c\": Group=%u Item=%u\n", ID, PrefixA, PrefixB, Group, Item );
	}
	 */

	// Update binding map before going ahead.
	updateBindingMap();
	
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
					return;
					break;

				case evtRET_REMOVE:
					break;
					
				default:
					BcBreakpoint;
					break;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// registerBinding
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
// unregisterBinding
void EvtPublisher::unsubscribeInternal( EvtID ID, const EvtBinding& Binding )
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
			if( (*Iter) == Binding )
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
	// Subscribe
	for( TBindingPairListIterator Iter = SubscribeList_.begin(); Iter != SubscribeList_.end(); ++Iter )
	{
		subscribeInternal( Iter->first, Iter->second );
	}

	SubscribeList_.clear();

	// Unsubscribe
	for( TBindingPairListIterator Iter = UnsubscribeList_.begin(); Iter != UnsubscribeList_.end(); ++Iter )
	{
		unsubscribeInternal( Iter->first, Iter->second );
	}

	UnsubscribeList_.clear();
}
