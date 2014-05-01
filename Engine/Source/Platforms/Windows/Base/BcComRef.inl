template< class _Ty >
inline void BcComRef< _Ty >::_acquireNew( IUnknown* pObject )
{
	pObject_ = pObject->isTypeOf< _Ty >() ? pObject : nullptr;
	if( pObject_ != nullptr )
	{
		pObject_->AddRef();
	}
}

template< class _Ty >
inline void BcComRef< _Ty >::_acquireNewReleaseOld( IUnknown* pObject )
{
	pObject = pObject->isTypeOf< _Ty >() ? pObject : nullptr;
	if( pObject != nullptr )
	{
		pObject->AddRef();
	}

	pObject_->Release();
	pObject_ = pObject;
}

template< class _Ty >
inline void BcComRef< _Ty >::_acquireAssign( IUnknown* pObject )
{
	pObject = pObject->isTypeOf< _Ty >() ? pObject : nullptr;
	if( pObject_ == nullptr )
	{
		_acquireNew( pObject );
	}
	else
	{
		_acquireNewReleaseOld( pObject );
	}
	pObject_ = pObject;
}

template< class _Ty >
inline void BcComRef< _Ty >::_releaseThis()
{
	if( pObject_ != nullptr )
	{
		pObject_->Release();
		pObject_ = nullptr;
	}
}

template< class _Ty >
inline BcComRef< _Ty >::BcComRef():
	pObject_( nullptr )
{
}

template< class _Ty >
inline BcComRef< _Ty >::BcComRef( const BcComRef& Other )
{
	_acquireNew( Other.pObject_ );
}

template< class _Ty >
inline BcComRef< _Ty >::BcComRef( IUnknown* pObject )
{
	_acquireNew( pObject );
}

template< class _Ty >
inline BcComRef< _Ty, _IsWeak >& BcComRef< _Ty, _IsWeak >::operator = ( const BcComRef& Other )
{
	_acquireAssign( Other.pObject_ );
	return (*this);
}

template< class _Ty >
inline BcComRef< _Ty >& BcComRef< _Ty, _IsWeak >::operator = ( IUnknown* pObject )
{
	_acquireAssign( pObject );
	return (*this);
}

template< class _Ty >
inline BcComRef< _Ty >::~BcComRef()
{
	_releaseThis();
}

template< class _Ty >
inline bool BcComRef< _Ty >::isValid() const
{
	return ( pObject_ != nullptr && ( ( ((BcU32)pObject_->Flags_) & (BcU32)IUnknown::Flags::MarkedForDeletion ) == 0 ) );
}
		
template< class _Ty >
inline BcComRef< _Ty >::operator _Ty* ()
{
	return static_cast< _Ty* >( pObject_ );
}

template< class _Ty >
inline BcComRef< _Ty >::operator const _Ty* () const
{
	return static_cast< const _Ty* >( pObject_ );
}

template< class _Ty >
inline _Ty* BcComRef< _Ty >::operator -> ()
{
	BcAssert( pObject_ != nullptr );
	return static_cast< _Ty* >( pObject_ );
}

template< class _Ty >
inline const _Ty* BcComRef< _Ty >::operator -> () const
{
	BcAssert( pObject_ != nullptr );
	return static_cast< const _Ty* >( pObject_ );
}

template< class _Ty >
inline _Ty** BcComRef< _Ty >::operator & ()
{
	return reinterpret_cast< _Ty*& >( &pObject_ );
}

template< class _Ty >
inline bool BcComRef< _Ty >::operator == ( const BcComRef& Other ) const
{
	return Other.pObject_ == pObject_;
}

template< class _Ty >
inline bool BcComRef< _Ty >::operator != ( const BcComRef& Other ) const
{
	return Other.pObject_ != pObject_;
}

template< class _Ty >
inline bool BcComRef< _Ty >::operator == ( _Ty* pObject ) const
{
	return pObject == pObject_;
}

template< class _Ty >
inline bool BcComRef< _Ty >::operator != ( _Ty* pObject ) const
{
	return pObject != pObject_;
}

template< class _Ty >
inline void BcComRef< _Ty >::reset()
{
	_releaseThis();
}
