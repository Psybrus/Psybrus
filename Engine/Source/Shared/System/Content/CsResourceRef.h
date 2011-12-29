/**************************************************************************
*
* File:		CsResourceRef.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Resource handle
*		
*
*
* 
**************************************************************************/

#ifndef __CSRESOURCEREF_H__
#define __CSRESOURCEREF_H__

//#include "CsResource.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class CsResource;

//////////////////////////////////////////////////////////////////////////
// CsResourceRef
template< typename _Ty = CsResource, bool _IsWeak = false >
class CsResourceRef
{
private:
	CsResource* pObject_;
	
private:
	BcForceInline void _acquireNew( CsResource* pObject )
	{
		pObject_ = pObject->isTypeOf< _Ty >() ? pObject : NULL;

		if( _IsWeak == false )
		{
			if( pObject_ != NULL )
			{
				pObject_->acquire();
			}
		}
	}
	
	BcForceInline void _acquireNewReleaseOld( CsResource* pObject )
	{
		pObject = pObject->isTypeOf< _Ty >() ? pObject : NULL;

		if( _IsWeak == false )
		{
			if( pObject != NULL )
			{
				pObject->acquire();
			}
	
			pObject_->release();
		}

		pObject_ = pObject;
	}

	BcForceInline void _acquireAssign( CsResource* pObject )
	{
		pObject = pObject->isTypeOf< _Ty >() ? pObject : NULL;

		if( _IsWeak == true || pObject_ == NULL )
		{
			_acquireNew( pObject );
		}
		else
		{
			_acquireNewReleaseOld( pObject );
		}
	}

	BcForceInline void _releaseThis()
	{
		if( _IsWeak == false )
		{
			if( pObject_ != NULL )
			{
				pObject_->release();
				pObject_ = NULL;
			}
		}
	}

public:
	BcForceInline CsResourceRef():
		pObject_( NULL )
	{
	}
	
	BcForceInline CsResourceRef( const CsResourceRef& Other )
	{
		_acquireNew( Other.pObject_ );
	}
	
	BcForceInline CsResourceRef( CsResource* pObject )
	{
		_acquireNew( pObject );
	}
	
	BcForceInline CsResourceRef& operator = ( const CsResourceRef& Other )
	{
		_acquireAssign( Other.pObject_ );
		return (*this);
	}
	
	BcForceInline CsResourceRef& operator = ( CsResource* pObject )
	{
		_acquireAssign( pObject );
		return (*this);
	}
	
	BcForceInline ~CsResourceRef()
	{
		_releaseThis();
	}
	
	BcForceInline BcBool isValid() const
	{
		return ( pObject_ != NULL );
	}
	
	BcForceInline BcBool isReady() const
	{
		return isValid() ? pObject_->isReady() : BcFalse;
	
	}

	BcForceInline operator _Ty* ()
	{
		return static_cast< _Ty* >( pObject_ );
	}
	
	BcForceInline _Ty* operator -> ()
	{
		return static_cast< _Ty* >( pObject_ );
	}
	
	BcForceInline BcBool operator == ( const CsResourceRef& Other ) const
	{
		return Other.pObject_ == pObject_;
	}
	
	BcForceInline BcBool operator != ( const CsResourceRef& Other ) const
	{
		return Other.pObject_ != pObject_;
	}

	BcForceInline BcBool operator == ( _Ty* pObject ) const
	{
		return pObject == pObject_;
	}
	
	BcForceInline BcBool operator != ( _Ty* pObject ) const
	{
		return pObject != pObject_;
	}
};

#endif
