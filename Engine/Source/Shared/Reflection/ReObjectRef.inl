template< class _Ty >
inline void ReObjectRef< _Ty >::_acquireNew( ReObject* pObject )
{
	pObject_ = pObject->isTypeOf< _Ty >() ? pObject : nullptr;

	if( pObject_ != nullptr )
	{
		pObject_->incRefCount();
	}
}

template< class _Ty >
inline void ReObjectRef< _Ty >::_acquireNewReleaseOld( ReObject* pObject )
{
	pObject = pObject->isTypeOf< _Ty >() ? pObject : nullptr;

	if( pObject != nullptr )
	{
		pObject->incRefCount();
	}

	pObject_->decRefCount();
	pObject_ = pObject;
}

template< class _Ty >
inline void ReObjectRef< _Ty >::_acquireAssign( ReObject* pObject )
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
}

template< class _Ty >
inline void ReObjectRef< _Ty >::_releaseThis()
{
	if( pObject_ != nullptr )
	{
		pObject_->decRefCount();
		pObject_ = nullptr;
	}
}

//static
template< class _Ty >
inline void ReObjectRef< _Ty >::assertPendingDeletion( const ReObject* pObject )
{
#if PSY_DEBUG
	if( pObject != nullptr )
	{
		BcAssert( ( pObject->Flags_ & (BcU32)Object::Flags::MarkedForDeletion ) == 0 );
	}
#endif // PSY_DEBUG
}

template< class _Ty >
inline ReObjectRef< _Ty >::ReObjectRef():
	pObject_( nullptr )
{
}

template< class _Ty >
inline ReObjectRef< _Ty >::ReObjectRef( const ReObjectRef& Other )
{
	assertPendingDeletion( Other.pObject_ );
	_acquireNew( Other.pObject_ );
}

template< class _Ty >
inline ReObjectRef< _Ty >::ReObjectRef( ReObject* pObject )
{
	assertPendingDeletion( pObject );
	_acquireNew( pObject );
}

template< class _Ty >
inline ReObjectRef< _Ty >& ReObjectRef< _Ty >::operator = ( const ReObjectRef& Other )
{
	assertPendingDeletion( Other.pObject_ );
	_acquireAssign( Other.pObject_ );
	return (*this);
}

template< class _Ty >
inline ReObjectRef< _Ty >& ReObjectRef< _Ty >::operator = ( ReObject* pObject )
{
	assertPendingDeletion( pObject );
	_acquireAssign( pObject );
	return (*this);
}

template< class _Ty >
inline ReObjectRef< _Ty >::~ReObjectRef()
{
	_releaseThis();
}

template< class _Ty >
inline bool ReObjectRef< _Ty >::isValid() const
{
	return ( pObject_ != nullptr && ( ( pObject_->Flags_ & ReObject::MarkedForDeletion ) == 0 ) );
}
		
template< class _Ty >
inline ReObjectRef< _Ty >::operator _Ty* ()
{
	assertPendingDeletion( pObject_ );
	return static_cast< _Ty* >( pObject_ );
}

template< class _Ty >
inline _Ty* ReObjectRef< _Ty >::operator -> ()
{
	BcAssert( pObject_ != nullptr );
	assertPendingDeletion( pObject_ );
	return static_cast< _Ty* >( pObject_ );
}

template< class _Ty >
inline bool ReObjectRef< _Ty >::operator == ( const ReObjectRef& Other ) const
{
	assertPendingDeletion( pObject_ );
	assertPendingDeletion( Other.pObject_ );
	return Other.pObject_ == pObject_;
}

template< class _Ty >
inline bool ReObjectRef< _Ty >::operator != ( const ReObjectRef& Other ) const
{
	assertPendingDeletion( pObject_ );
	assertPendingDeletion( Other.pObject_ );
	return Other.pObject_ != pObject_;
}

template< class _Ty >
inline bool ReObjectRef< _Ty >::operator == ( _Ty* pObject ) const
{
	assertPendingDeletion( pObject_ );
	assertPendingDeletion( Other.pObject_ );
	return pObject == pObject_;
}

template< class _Ty >
inline bool ReObjectRef< _Ty >::operator != ( _Ty* pObject ) const
{
	assertPendingDeletion( pObject_ );
	assertPendingDeletion( Other.pObject_ );
	return pObject != pObject_;
}

template< class _Ty >
inline void ReObjectRef< _Ty >::reset()
{
	_releaseThis();
}
