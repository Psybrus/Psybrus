template< class _Ty >
inline void ObjectRef< _Ty >::_acquireNew( Object* pObject )
{
	pObject_ = pObject->isTypeOf< _Ty >() ? pObject : nullptr;

	if( pObject_ != nullptr )
	{
		pObject_->incRefCount();
	}
}

template< class _Ty >
inline void ObjectRef< _Ty >::_acquireNewReleaseOld( Object* pObject )
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
inline void ObjectRef< _Ty >::_acquireAssign( Object* pObject )
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
inline void ObjectRef< _Ty >::_releaseThis()
{
	if( pObject_ != nullptr )
	{
		pObject_->decRefCount();
		pObject_ = nullptr;
	}
}

//static
template< class _Ty >
inline void ObjectRef< _Ty >::assertPendingDeletion( const Object* pObject )
{
#if PSY_DEBUG
	if( pObject != nullptr )
	{
		BcAssert( ( pObject->Flags_ & (BcU32)Object::Flags::MarkedForDeletion ) == 0 );
	}
#endif // PSY_DEBUG
}

template< class _Ty >
inline ObjectRef< _Ty >::ObjectRef():
	pObject_( nullptr )
{
}

template< class _Ty >
inline ObjectRef< _Ty >::ObjectRef( const ObjectRef& Other )
{
	assertPendingDeletion( Other.pObject_ );
	_acquireNew( Other.pObject_ );
}

template< class _Ty >
inline ObjectRef< _Ty >::ObjectRef( Object* pObject )
{
	assertPendingDeletion( pObject );
	_acquireNew( pObject );
}

template< class _Ty >
inline ObjectRef< _Ty >& ObjectRef< _Ty >::operator = ( const ObjectRef& Other )
{
	assertPendingDeletion( Other.pObject_ );
	_acquireAssign( Other.pObject_ );
	return (*this);
}

template< class _Ty >
inline ObjectRef< _Ty >& ObjectRef< _Ty >::operator = ( Object* pObject )
{
	assertPendingDeletion( pObject );
	_acquireAssign( pObject );
	return (*this);
}

template< class _Ty >
inline ObjectRef< _Ty >::~ObjectRef()
{
	_releaseThis();
}

template< class _Ty >
inline bool ObjectRef< _Ty >::isValid() const
{
	return ( pObject_ != nullptr && ( ( pObject_->Flags_ & Object::MarkedForDeletion ) == 0 ) );
}
		
template< class _Ty >
inline ObjectRef< _Ty >::operator _Ty* ()
{
	assertPendingDeletion( pObject_ );
	return static_cast< _Ty* >( pObject_ );
}

template< class _Ty >
inline _Ty* ObjectRef< _Ty >::operator -> ()
{
	BcAssert( pObject_ != nullptr );
	assertPendingDeletion( pObject_ );
	return static_cast< _Ty* >( pObject_ );
}

template< class _Ty >
inline bool ObjectRef< _Ty >::operator == ( const ObjectRef& Other ) const
{
	assertPendingDeletion( pObject_ );
	assertPendingDeletion( Other.pObject_ );
	return Other.pObject_ == pObject_;
}

template< class _Ty >
inline bool ObjectRef< _Ty >::operator != ( const ObjectRef& Other ) const
{
	assertPendingDeletion( pObject_ );
	assertPendingDeletion( Other.pObject_ );
	return Other.pObject_ != pObject_;
}

template< class _Ty >
inline bool ObjectRef< _Ty >::operator == ( _Ty* pObject ) const
{
	assertPendingDeletion( pObject_ );
	assertPendingDeletion( Other.pObject_ );
	return pObject == pObject_;
}

template< class _Ty >
inline bool ObjectRef< _Ty >::operator != ( _Ty* pObject ) const
{
	assertPendingDeletion( pObject_ );
	assertPendingDeletion( Other.pObject_ );
	return pObject != pObject_;
}

template< class _Ty >
inline void ObjectRef< _Ty >::reset()
{
	_releaseThis();
}
