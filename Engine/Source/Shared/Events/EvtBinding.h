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

#include "Events/EvtEvent.h"

#include "Base/BcMemory.h"

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
	typedef eEvtReturn(*BaseSignature)( EvtID, const EvtBaseEvent& );
	typedef std::function< eEvtReturn( EvtID, const EvtBaseEvent& ) > BaseFunction;
	BaseFunction Function_;
	void* Owner_;

	/**
	* Construct binding from delegate.
	*/
	BcForceInline explicit EvtBinding( void* Owner, const BaseFunction& Function )
	{
		Function_ = Function;
		Owner_ = Owner;
	}

	/**
	* Call the delegate contained within.
	*/
	BcForceInline eEvtReturn operator()( EvtID ID, const EvtBaseEvent& Event )
	{	
		return Function_( ID, Event );
	}

	/**
	 * Get owner.
	 */
	BcForceInline void* getOwner()
	{	
		return Owner_;
	}

};


#endif
