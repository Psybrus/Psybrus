/**************************************************************************
*
* File:		EvtBinding.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event binding class.
*		
*		
*
* 
**************************************************************************/

#ifndef __EVTBINDING_H__
#define __EVTBINDING_H__

#include "EvtEvent.h"

#include "BcMemory.h"

////////////////////////////////////////////////////////////////////////////////
/**	\class EvtBinding
*	\brief Binding wrapper.
*
*	This filthy little class is used to allow binding of delegates with specific event
*	types in their event parameter field. This removes the need for the user to perform
*	the cast themselves.
*/
struct EvtBinding
{
	typedef BcDelegate< eEvtReturn(*)( EvtID, const EvtBaseEvent& ) > BaseDelegate;
	BaseDelegate Delegate_;

	/**
	* Construct binding from delegate.
	*/
	template< typename _Ty >
	BcForceInline explicit EvtBinding( BcDelegate< eEvtReturn(*)( EvtID, const _Ty& ) >& ExternalDelegate )
	{
		Delegate_ = *( reinterpret_cast< BaseDelegate* >( &ExternalDelegate ) );
	}

	/**
	* Comparison ==
	*/
	BcForceInline BcBool operator == ( const EvtBinding& Other ) const
	{
		return BcMemCompare( &Delegate_, &Other.Delegate_, sizeof( Delegate_ ) );
	}

	/**
	* Comparison !=
	*/
	BcForceInline BcBool operator !=( const EvtBinding& Other ) const
	{
		return !( *this == Other );
	}

	/**
	* Call the delegate contained within.
	*/
	BcForceInline eEvtReturn operator()( EvtID ID, const EvtBaseEvent& Event )
	{	
		return Delegate_( ID, Event );
	}
};


#endif
