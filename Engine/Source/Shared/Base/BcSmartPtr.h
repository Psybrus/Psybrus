/**************************************************************************
*
* File:		BcSmartPtr.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		Smart pointer.
*		
*
*
* 
**************************************************************************/

#ifndef __BCSMARTPTR_H__
#define __BCSMARTPTR_H__

#include "BcTypes.h"
#include "BcDebug.h"

//////////////////////////////////////////////////////////////////////////
// BcSmartPtr
template< typename _Ty >
class BcSmartPtr
{
public:
	typedef _Ty ptr_type;

	BcForceInline BcSmartPtr()
	{
		pObject_ = NULL;
	}

	BcForceInline BcSmartPtr( const BcSmartPtr< _Ty >& Other )
	{
		pObject_ = Other.pObject_;

		if( pObject_ != NULL )
		{
			pObject_->incRef();
		}
	}

	template< typename _U >
	BcForceInline BcSmartPtr( const BcSmartPtr< _U >& Other )
	{
		if( Other.get() != NULL )
		{
			BcAssertMsg( static_cast< _Ty* >( Other.get() ) != NULL, "Invalid typecast." );

			pObject_ = dynamic_cast< _Ty* >( Other.get() );

			if( pObject_ != NULL )
			{
				pObject_->incRef();
			}
		}
		else
		{
			pObject_ = NULL;
		}
	}
	
	BcForceInline BcSmartPtr( _Ty* pObject )
	{
		pObject_ = pObject;

		if( pObject != NULL )
		{
			pObject_->incRef();
		}
	}

	BcInline BcSmartPtr& operator = ( const BcSmartPtr< _Ty >& Other )
	{
		if( pObject_ == NULL )
		{
			pObject_ = Other.pObject_;

			if( pObject_ != NULL )
			{
				pObject_->incRef();
			}
		}
		else
		{
			if( Other.pObject_ != NULL )
			{
				Other.pObject_->incRef();
			}

			pObject_->decRef();
			pObject_ = Other.pObject_;
		}

		return *this;
	}

	BcInline BcSmartPtr& operator = ( _Ty* pObject )
	{
		if( pObject_ == NULL )
		{
			pObject_ = pObject;

			if( pObject_ != NULL )
			{
				pObject_->incRef();
			}
		}
		else
		{
			if( pObject != NULL )
			{
				pObject->incRef();
			}

			pObject_->decRef();
			pObject_ = pObject;
		}

		return *this;
	}

	BcForceInline ~BcSmartPtr()
	{
		if( pObject_ != NULL )
		{
			pObject_->decRef();
			pObject_ = NULL;
		}
	}

	BcForceInline _Ty* get() const
	{
		return pObject_;
	}

	BcForceInline BcBool isNull() const
	{
		return pObject_ == NULL;
	}

	BcForceInline void release()
	{
		*this = NULL;
	}

	BcForceInline _Ty* operator ->() const
	{
		return pObject_;
	}

	BcForceInline _Ty& operator *() const
	{
		return *pObject_;
	}

	BcForceInline BcBool operator == ( _Ty* pObject ) const
	{
		return pObject == pObject_;
	}

	BcForceInline BcBool operator != ( _Ty* pObject ) const
	{
		return pObject != pObject_;
	}

	BcForceInline BcBool operator == ( const BcSmartPtr< _Ty >& Other ) const
	{
		return Other.pObject_ == pObject_;
	}

	BcForceInline BcBool operator != ( const BcSmartPtr< _Ty >& Other ) const
	{
		return Other.pObject_ != pObject_;
	}

private:
	_Ty*				pObject_;
};

//////////////////////////////////////////////////////////////////////////
// BcSmartPtrCount
class BcSmartPtrCount
{
public:
	BcForceInline BcSmartPtrCount()
	{
		RefCount_ = 0;
		PendingDeletion_ = BcFalse;
	}

	BcForceInline virtual ~BcSmartPtrCount()
	{
		BcAssert( RefCount_ == 0 );
	}

	BcForceInline void incRef()
	{
		++RefCount_;
	}

	BcForceInline void decRef()
	{
		--RefCount_;
		if( RefCount_ == 0 && PendingDeletion_ == BcFalse )
		{
			PendingDeletion_ = BcTrue;
			delete this;
		}
	}

protected:
	BcU32				RefCount_;
	BcBool				PendingDeletion_;
};

#endif
