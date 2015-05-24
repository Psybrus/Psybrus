#pragma once

#include "Base/BcTypes.h"
#include "Base/BcDebug.h"

#include <limits>
#include <type_traits>

//////////////////////////////////////////////////////////////////////////
// @brief Relative pointer.
// Stores an offset from 'this' to the data it points to rather than pointer.
// This will allow it to be relocatable, as well as being able to reduce structure
// size when the data pointed to is known to be close to this.
template< typename _OffsetType, typename _Ty >
class BcRelativePtr
{
private:
	static const size_t OFFSET_TYPE_BITS = sizeof( _OffsetType ) * 8;
	static const ptrdiff_t OFFSET_TYPE_MIN = std::numeric_limits< _OffsetType >::min();
	static const ptrdiff_t OFFSET_TYPE_MAX = std::numeric_limits< _OffsetType >::max() - 1;
	static const _OffsetType NULLPTR_VALUE = OFFSET_TYPE_MAX + 1;

	static _OffsetType CalculateOffset( ptrdiff_t Ref, ptrdiff_t Ptr );
	static ptrdiff_t CalculatePtr( ptrdiff_t Ref, _OffsetType Offset );

public:
	BcRelativePtr();
	BcRelativePtr( std::nullptr_t );
	BcRelativePtr( _Ty* Ptr );
	BcRelativePtr& operator = ( _Ty* Ptr );
	BcRelativePtr& operator = ( BcRelativePtr&& Other );

	/**
	 * Offset.
	 */
	_OffsetType offset() const;

	/**
	 * Get raw pointer.
	 */
	_Ty* get();

	/**
	 * Get raw pointer.
	 */
	const _Ty* get() const;

	/**
	 * Implicit cast to pointer.
	 */
	_Ty* operator * ();

	/**
	 * Implicit cast to pointer.
	 */
	const _Ty* operator * () const;

	/**
	 * Dereference pointer.
	 */
	_Ty* operator -> ();

	/**
	 * Dereference pointer.
	 */
	const _Ty* operator -> () const;

	/**
	 * @return True if equal to @a Other relative pointer.
	 */
	bool operator == ( const BcRelativePtr& Other );

	/**
	 * @return True if not equal to @a Other relative pointer.
	 */
	bool operator != ( const BcRelativePtr& Other );

private:
	_OffsetType Offset_;
};

//////////////////////////////////////////////////////////////////////////
// Aliases
template< typename _Ty >
using BcRelativePtrU8 = BcRelativePtr< BcU8, _Ty >;

template< typename _Ty >
using BcRelativePtrU16 = BcRelativePtr< BcU16, _Ty >;

template< typename _Ty >
using BcRelativePtrU32 = BcRelativePtr< BcU32, _Ty >;

template< typename _Ty >
using BcRelativePtrS8 = BcRelativePtr< BcS8, _Ty >;

template< typename _Ty >
using BcRelativePtrS16 = BcRelativePtr< BcS16, _Ty >;

template< typename _Ty >
using BcRelativePtrS32 = BcRelativePtr< BcS32, _Ty >;

#include "Base/BcRelativePtr.inl"

