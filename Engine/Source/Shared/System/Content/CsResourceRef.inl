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
BcForceInline void CsResourceRef< _Ty, _IsWeak >::_acquireAssign( CsResource* pObject )
{
    pObject_ = pObject->isTypeOf< _Ty >() ? pObject : NULL;
}

template< class _Ty, bool _IsWeak >
BcForceInline CsResourceRef< _Ty, _IsWeak >::CsResourceRef():
    pObject_( NULL )
{
}

template< class _Ty, bool _IsWeak >
BcForceInline CsResourceRef< _Ty, _IsWeak >::CsResourceRef( const CsResourceRef& Other )
{
    _acquireAssign( Other.pObject_ );
}

template< class _Ty, bool _IsWeak >
BcForceInline CsResourceRef< _Ty, _IsWeak >::CsResourceRef( CsResource* pObject )
{
    _acquireAssign( pObject );
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
BcForceInline CsResourceRef< _Ty, _IsWeak >::operator _Ty* ()
{
    return static_cast< _Ty* >( pObject_ );
}

template< class _Ty, bool _IsWeak >
BcForceInline CsResourceRef< _Ty, _IsWeak >::operator const _Ty* () const
{
    return static_cast< const _Ty* >( pObject_ );
}

template< class _Ty, bool _IsWeak >
BcForceInline _Ty* CsResourceRef< _Ty, _IsWeak >::operator -> ()
{
    return static_cast< _Ty* >( pObject_ );
}

template< class _Ty, bool _IsWeak >
BcForceInline const _Ty* CsResourceRef< _Ty, _IsWeak >::operator -> () const
{
    return static_cast< const _Ty* >( pObject_ );
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
