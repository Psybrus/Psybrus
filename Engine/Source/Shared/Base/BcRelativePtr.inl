template< typename _OffsetType, typename _Ty >
_OffsetType BcRelativePtr< _OffsetType, _Ty >::CalculateOffset( ptrdiff_t Ref, ptrdiff_t Ptr )
{
	const ptrdiff_t Diff = Ptr - Ref;
	auto RetVal = static_cast< _OffsetType >( Diff );
	BcAssertMsg( Diff >= OFFSET_TYPE_MIN && Diff <= OFFSET_TYPE_MAX, 
		"Ptr is too far out of range (%i, min/max is %i/%i)", Diff, OFFSET_TYPE_MIN, OFFSET_TYPE_MAX );
	BcAssertMsg( Diff != NULLPTR_VALUE,
		"Offset resolves to NULLPTR_VALUE." );
	return RetVal;
}

template< typename _OffsetType, typename _Ty >
ptrdiff_t BcRelativePtr< _OffsetType, _Ty >::CalculatePtr( ptrdiff_t Ref, _OffsetType Offset )
{
	auto RefOffset = Ref + static_cast< ptrdiff_t >( Offset ); 
	return Offset != NULLPTR_VALUE ? RefOffset : 0;
}

template< typename _OffsetType, typename _Ty >
BcRelativePtr< _OffsetType, _Ty >::BcRelativePtr():
	Offset_( NULLPTR_VALUE )
{

}

template< typename _OffsetType, typename _Ty >
BcRelativePtr< _OffsetType, _Ty >::BcRelativePtr( std::nullptr_t ):
	Offset_( NULLPTR_VALUE )
{

}

template< typename _OffsetType, typename _Ty >
BcRelativePtr< _OffsetType, _Ty >::BcRelativePtr( _Ty* Ptr ):
	Offset_( CalculateOffset( this, Ptr ) )
{

}

template< typename _OffsetType, typename _Ty >
BcRelativePtr< _OffsetType, _Ty > & BcRelativePtr< _OffsetType, _Ty >::operator = ( _Ty* Ptr )
{
	Offset_ = CalculateOffset( 
		reinterpret_cast< ptrdiff_t >( this ), 
		reinterpret_cast< ptrdiff_t >( Ptr ) );
	return *this;
}

template< typename _OffsetType, typename _Ty >
BcRelativePtr< _OffsetType, _Ty >& BcRelativePtr< _OffsetType, _Ty >::operator = ( BcRelativePtr&& Other )
{
	std::swap( Offset_, Other.Offset_ );
	return *this;
}

template< typename _OffsetType, typename _Ty >
_OffsetType BcRelativePtr< _OffsetType, _Ty >::offset() const
{
	return Offset_;
}

template< typename _OffsetType, typename _Ty >
_Ty* BcRelativePtr< _OffsetType, _Ty >::get()
{
	auto RetVal = CalculatePtr( reinterpret_cast< ptrdiff_t >( this ), Offset_ );
	return reinterpret_cast< _Ty* >( RetVal );
}

template< typename _OffsetType, typename _Ty >
const _Ty* BcRelativePtr< _OffsetType, _Ty >::get() const
{
	auto RetVal = CalculatePtr( reinterpret_cast< ptrdiff_t >( this ), Offset_ );
	return reinterpret_cast< const _Ty* >( RetVal );
}

template< typename _OffsetType, typename _Ty >
_Ty* BcRelativePtr< _OffsetType, _Ty >::operator * ()
{
	return get();
}

template< typename _OffsetType, typename _Ty >
const _Ty* BcRelativePtr< _OffsetType, _Ty >::operator * () const
{
	return get();
}

template< typename _OffsetType, typename _Ty >
_Ty* BcRelativePtr< _OffsetType, _Ty >::operator -> ()
{
	return get();
}

template< typename _OffsetType, typename _Ty >
const _Ty* BcRelativePtr< _OffsetType, _Ty >::operator -> () const
{
	return get();
}

template< typename _OffsetType, typename _Ty >
bool BcRelativePtr< _OffsetType, _Ty >::operator == ( const BcRelativePtr& Other )
{
	return get() == Other.get();
}

template< typename _OffsetType, typename _Ty >
bool BcRelativePtr< _OffsetType, _Ty >::operator != ( const BcRelativePtr& Other )
{
	return get() != Other.get();
}
