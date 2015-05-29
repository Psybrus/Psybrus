#pragma once

#include "Base/BcTypes.h"
#include "Base/BcDebug.h"

#include <limits>
#include <type_traits>

#if COMPILER_MSVC
// Fallback to boost as VS doesn't support const_expr.
#include <boost/integer.hpp>
#endif

//////////////////////////////////////////////////////////////////////////
// @brief Relative pointer.
// Stores an offset from 'this' to the data it points to rather than pointer.
// This will allow it to be relocatable, as well as being able to reduce structure
// size when the data pointed to is known to be close to this.
template< typename _OffsetType, typename _Ty >
class BcRelativePtr
{
private:
#if COMPILER_MSVC
	static const ptrdiff_t OFFSET_TYPE_MIN = boost::integer_traits< _OffsetType >::const_min;
	static const ptrdiff_t OFFSET_TYPE_MAX = boost::integer_traits< _OffsetType >::const_max - 1;
#else
	static const ptrdiff_t OFFSET_TYPE_MIN = std::numeric_limits< _OffsetType >::min();
	static const ptrdiff_t OFFSET_TYPE_MAX = std::numeric_limits< _OffsetType >::max() - 1;
#endif

	static const size_t OFFSET_TYPE_BITS = sizeof( _OffsetType ) * 8;
	static const _OffsetType NULLPTR_VALUE = static_cast< _OffsetType >( OFFSET_TYPE_MAX + 1 );

	static _OffsetType CalculateOffset( ptrdiff_t Ref, ptrdiff_t Ptr );
	static ptrdiff_t CalculatePtr( ptrdiff_t Ref, _OffsetType Offset );

public:
	BcRelativePtr();
	BcRelativePtr( std::nullptr_t );
	BcRelativePtr( BcRelativePtr&& Other );
	BcRelativePtr( const BcRelativePtr& Other ) = delete;
	BcRelativePtr& operator = ( BcRelativePtr&& Other );
	BcRelativePtr& operator = ( const BcRelativePtr& Other ) = delete;

	/**
	 * Reset.
	 */
	void reset( _Ty* Ptr );

	/**
	 * Offset.
	 */
	_OffsetType offset() const;

	/**
	 * Get raw pointer.
	 */
	_Ty* get() const;

	/**
	 * Dereference pointer.
	 */
	_Ty& operator * () const;

	/**
	 * Dereference pointer.
	 */
	_Ty* operator -> () const;

	/**
	 * Indexed getter.
	 * Will not do bounds checking.
	 */
	_Ty& operator [] ( size_t Idx ) const;

	/**
	 * @return True if equal to @a Other relative pointer.
	 */
	bool operator == ( const BcRelativePtr& Other ) const;

	/**
	 * @return True if not equal to @a Other relative pointer.
	 */
	bool operator != ( const BcRelativePtr& Other ) const;

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

