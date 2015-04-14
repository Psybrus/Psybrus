template< class _Ty, bool _IsWeak >
inline void ReObjectRef< _Ty, _IsWeak >::_acquireNew( ReObject* pObject )
{
	pObject_ = ( pObject != nullptr && pObject->isTypeOf< _Ty >() ) ? pObject : nullptr;
#if REFLECTION_ENABLE_GC
	if( !_IsWeak )
	{
		if( pObject_ != nullptr )
		{
			pObject_->incRefCount();
		}
	}
#endif
}

template< class _Ty, bool _IsWeak >
inline void ReObjectRef< _Ty, _IsWeak >::_acquireNewReleaseOld( ReObject* pObject )
{
	pObject = ( pObject != nullptr && pObject->isTypeOf< _Ty >() ) ? pObject : nullptr;
#if REFLECTION_ENABLE_GC
	if( !_IsWeak )
	{
		if( pObject != nullptr )
		{
			pObject->incRefCount();
		}

		pObject_->decRefCount();
	}
#endif
	pObject_ = pObject;
}

template< class _Ty, bool _IsWeak >
inline void ReObjectRef< _Ty, _IsWeak >::_acquireAssign( ReObject* pObject )
{
	pObject = ( pObject != nullptr && pObject->isTypeOf< _Ty >() ) ? pObject : nullptr;
#if REFLECTION_ENABLE_GC
	if( !_IsWeak )
	{
		if( pObject_ == nullptr )
		{
			_acquireNew( pObject );
		}
		else
		{
			_acquireNewReleaseOld( pObject );
		}
	}
#endif
	pObject_ = pObject;
}

template< class _Ty, bool _IsWeak >
inline void ReObjectRef< _Ty, _IsWeak >::_releaseThis()
{
	if( pObject_ != nullptr )
	{
#if REFLECTION_ENABLE_GC
		if( !_IsWeak )
		{
			pObject_->decRefCount();
		}
#endif
		pObject_ = nullptr;
	}
}

//static
template< class _Ty, bool _IsWeak >
inline void ReObjectRef< _Ty, _IsWeak >::assertPendingDeletion( const ReObject* pObject )
{
#if defined( PSY_DEBUG ) && REFLECTION_ENABLE_GC
	if( pObject != nullptr )
	{
		BcAssert( ( pObject->Flags_ & (BcU32)Object::Flags::MarkedForDeletion ) == 0 );
	}
#endif // PSY_DEBUG
}

template< class _Ty, bool _IsWeak >
inline ReObjectRef< _Ty, _IsWeak >::ReObjectRef():
	pObject_( nullptr )
{
}

template< class _Ty, bool _IsWeak >
inline ReObjectRef< _Ty, _IsWeak >::ReObjectRef( const ReObjectRef& Other )
{
	assertPendingDeletion( Other.pObject_ );
	_acquireNew( Other.pObject_ );
}

template< class _Ty, bool _IsWeak >
inline ReObjectRef< _Ty, _IsWeak >::ReObjectRef( ReObject* pObject )
{
	assertPendingDeletion( pObject );
	_acquireNew( pObject );
}

template< class _Ty, bool _IsWeak >
inline ReObjectRef< _Ty, _IsWeak >& ReObjectRef< _Ty, _IsWeak >::operator = ( const ReObjectRef& Other )
{
	assertPendingDeletion( Other.pObject_ );
	_acquireAssign( Other.pObject_ );
	return (*this);
}

template< class _Ty, bool _IsWeak >
inline ReObjectRef< _Ty, _IsWeak >& ReObjectRef< _Ty, _IsWeak >::operator = ( ReObject* pObject )
{
	assertPendingDeletion( pObject );
	_acquireAssign( pObject );
	return (*this);
}

template< class _Ty, bool _IsWeak >
inline ReObjectRef< _Ty, _IsWeak >::~ReObjectRef()
{
	_releaseThis();
}

template< class _Ty, bool _IsWeak >
inline bool ReObjectRef< _Ty, _IsWeak >::isValid() const
{
	return ( pObject_ != nullptr && ( ( ((BcU32)pObject_->Flags_) & (BcU32)ReObject::Flags::MarkedForDeletion ) == 0 ) );
}
		
template< class _Ty, bool _IsWeak >
inline ReObjectRef< _Ty, _IsWeak >::operator _Ty* ()
{
	assertPendingDeletion( pObject_ );
	return static_cast< _Ty* >( pObject_ );
}

template< class _Ty, bool _IsWeak >
inline ReObjectRef< _Ty, _IsWeak >::operator const _Ty* () const
{
	assertPendingDeletion( pObject_ );
	return static_cast< const _Ty* >( pObject_ );
}

template< class _Ty, bool _IsWeak >
inline _Ty* ReObjectRef< _Ty, _IsWeak >::operator -> ()
{
	BcAssert( pObject_ != nullptr );
	assertPendingDeletion( pObject_ );
	return static_cast< _Ty* >( pObject_ );
}

template< class _Ty, bool _IsWeak >
inline const _Ty* ReObjectRef< _Ty, _IsWeak >::operator -> () const
{
	BcAssert( pObject_ != nullptr );
	assertPendingDeletion( pObject_ );
	return static_cast< const _Ty* >( pObject_ );
}

template< class _Ty, bool _IsWeak >
inline bool ReObjectRef< _Ty, _IsWeak >::operator == ( const ReObjectRef& Other ) const
{
	assertPendingDeletion( pObject_ );
	assertPendingDeletion( Other.pObject_ );
	return Other.pObject_ == pObject_;
}

template< class _Ty, bool _IsWeak >
inline bool ReObjectRef< _Ty, _IsWeak >::operator != ( const ReObjectRef& Other ) const
{
	assertPendingDeletion( pObject_ );
	assertPendingDeletion( Other.pObject_ );
	return Other.pObject_ != pObject_;
}

template< class _Ty, bool _IsWeak >
inline bool ReObjectRef< _Ty, _IsWeak >::operator == ( _Ty* pObject ) const
{
	assertPendingDeletion( pObject_ );
	assertPendingDeletion( pObject );
	return pObject == pObject_;
}

template< class _Ty, bool _IsWeak >
inline bool ReObjectRef< _Ty, _IsWeak >::operator != ( _Ty* pObject ) const
{
	assertPendingDeletion( pObject_ );
	assertPendingDeletion( pObject );
	return pObject != pObject_;
}

template< class _Ty, bool _IsWeak >
inline void ReObjectRef< _Ty, _IsWeak >::reset()
{
	_releaseThis();
}
