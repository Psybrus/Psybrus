/**************************************************************************
*
* File:		CsResourceRef.inl
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*		Resource handle
*
*
*
*
**************************************************************************/

template< class _Ty, bool _IsWeak >
BcForceInline void CsResourceRef< _Ty, _IsWeak >::_acquireNew( CsResource* pObject )
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

template< class _Ty, bool _IsWeak >
BcForceInline void CsResourceRef< _Ty, _IsWeak >::_acquireNewReleaseOld( CsResource* pObject )
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

template< class _Ty, bool _IsWeak >
BcForceInline void CsResourceRef< _Ty, _IsWeak >::_acquireAssign( CsResource* pObject )
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

template< class _Ty, bool _IsWeak >
BcForceInline void CsResourceRef< _Ty, _IsWeak >::_releaseThis()
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

template< class _Ty, bool _IsWeak >
BcForceInline CsResourceRef< _Ty, _IsWeak >::CsResourceRef():
    pObject_( NULL )
{
}

template< class _Ty, bool _IsWeak >
BcForceInline CsResourceRef< _Ty, _IsWeak >::CsResourceRef( const CsResourceRef& Other )
{
    _acquireNew( Other.pObject_ );
}

template< class _Ty, bool _IsWeak >
BcForceInline CsResourceRef< _Ty, _IsWeak >::CsResourceRef( CsResource* pObject )
{
    _acquireNew( pObject );
}

template< class _Ty, bool _IsWeak >
BcForceInline CsResourceRef< _Ty, _IsWeak >& CsResourceRef< _Ty, _IsWeak >::operator = ( const CsResourceRef& Other )
{
    _acquireAssign( Other.pObject_ );
    return (*this);
}

template< class _Ty, bool _IsWeak >
BcForceInline CsResourceRef< _Ty, _IsWeak >& CsResourceRef< _Ty, _IsWeak >::operator = ( CsResource* pObject )
{
    _acquireAssign( pObject );
    return (*this);
}

template< class _Ty, bool _IsWeak >
BcForceInline CsResourceRef< _Ty, _IsWeak >::~CsResourceRef()
{
    _releaseThis();
}

template< class _Ty, bool _IsWeak >
BcForceInline BcBool CsResourceRef< _Ty, _IsWeak >::isValid() const
{
    return ( pObject_ != NULL );
}

template< class _Ty, bool _IsWeak >
BcForceInline BcBool CsResourceRef< _Ty, _IsWeak >::isReady() const
{
    return isValid() ? pObject_->isReady() : BcFalse;
}

template< class _Ty, bool _IsWeak >
BcForceInline BcU32 CsResourceRef< _Ty, _IsWeak >::refCount() const
{
	BcAssert( pObject_ != NULL );
    return pObject_->refCount();
}

template< class _Ty, bool _IsWeak >
BcForceInline CsResourceRef< _Ty, _IsWeak >::operator _Ty* ()
{
    return static_cast< _Ty* >( pObject_ );
}

template< class _Ty, bool _IsWeak >
BcForceInline _Ty* CsResourceRef< _Ty, _IsWeak >::operator -> ()
{
    return static_cast< _Ty* >( pObject_ );
}

template< class _Ty, bool _IsWeak >
BcForceInline BcBool CsResourceRef< _Ty, _IsWeak >::operator == ( const CsResourceRef& Other ) const
{
    return Other.pObject_ == pObject_;
}

template< class _Ty, bool _IsWeak >
BcForceInline BcBool CsResourceRef< _Ty, _IsWeak >::operator != ( const CsResourceRef& Other ) const
{
    return Other.pObject_ != pObject_;
}

template< class _Ty, bool _IsWeak >
BcForceInline BcBool CsResourceRef< _Ty, _IsWeak >::operator == ( _Ty* pObject ) const
{
    return pObject == pObject_;
}

template< class _Ty, bool _IsWeak >
BcForceInline BcBool CsResourceRef< _Ty, _IsWeak >::operator != ( _Ty* pObject ) const
{
    return pObject != pObject_;
}
