/**************************************************************************
*
* File:		BcGlobal.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __BCGLOBAL_H__
#define __BCGLOBAL_H__

#include "Base/BcTypes.h"

//////////////////////////////////////////////////////////////////////////
/** \class BcGlobal
*	\brief Templated global accessor.
*
*	Provides a global access point where there are potentially going to be
*	multiple valid implementations, but only 1 should ever exist at any time.			<br/>
*	It is your responsibility to delete it when it is time to go.
*
*/
template < class _Ty >
class BcGlobal
{
public:
	BcGlobal()
	{
#if PSY_DEBUG
		// No assert in here, used by logging system.
		if( pImpl_ != NULL )
		{
			BcBreakpoint;
		}
#endif
		pImpl_ = (_Ty*)this;
	}

	virtual ~BcGlobal()
	{
#if PSY_DEBUG
		// No assert in here, used by logging system.
		if( pImpl_ != (_Ty*)this )
		{
			BcBreakpoint;
		}
#endif
		pImpl_ = NULL;
	}
	
	/**
	*	Get the implementation as it's interface.
	*	@return Pointer to interface implementation.
	*/
	BcForceInline static _Ty* pImpl()
	{
		return pImpl_;
	}

	/**
	*	Get the implementation cast as something else.</br>
	*	Template specified must be a valid type, there is nothing
	*	to catch a bad cast here!
	*	@return Pointer to implementation.
	*/
	template< class _Uy >
	BcForceInline static _Uy* pImpl()
	{
		return static_cast< _Uy* >( pImpl_ );
	}
	
private:
	static _Ty* pImpl_;
};

//////////////////////////////////////////////////////////////////////////
// pImpl_
template < class _Ty >
_Ty* BcGlobal< _Ty >::pImpl_ = NULL;

#endif

