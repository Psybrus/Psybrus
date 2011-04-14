/**************************************************************************
*
* File:		BcAtomic.h
* Author: 	Neil Richardson 
* Ver/Date:	
*			
* Description:
*		Atomic data structures
*		
*		
* 
**************************************************************************/

#ifndef __BCATOMIC_H__
#define __BCATOMIC_H__

#include "BcTypes.h"

//////////////////////////////////////////////////////////////////////////
/**	\class BcAtomic
*	\brief Atomic data wrapper
*
*	Used to perform simple atomic operations on a type of up to
*	4 bytes in size. Typically used for ref counts, and lockless
*	algorithms.
*/
template< typename _Ty >
class BcAtomic
{
public:
	BcAtomic();
	BcAtomic( const BcAtomic< _Ty >& Value );
	BcAtomic( _Ty Value );
	
	/**
	* Assign from another atomic value.
	*/
	_Ty operator = ( const BcAtomic< _Ty >& Value );

	/**
	* Assign from templated value.
	*/
	_Ty operator = ( _Ty Value );
	
	/**
	* Cast operator for templated type.
	*/
	operator _Ty() const;
	
	/**
	* Pre increment.
	*/
	_Ty operator ++ ();

	/**
	* Pre decrement.
	*/
	_Ty operator -- ();
	
	/**
	* Post increment.
	*/
	_Ty operator ++ ( int );

	/**
	* Post decrement.
	*/
	_Ty operator -- ( int );
	
	/**
	* Exchange.
	* @param Value Value to set to.
	* @return Old value.
	*/
	_Ty exchange( _Ty Value );
		
	/**
	* Compare and exchange.
	* @param Exchange Value to store if Comparand matches current value.
	* @param Comparand Value to compare current value against.
	* @return Original Value.
	*/
	_Ty compareExchange( _Ty Exchange, _Ty Comparand );
	
private:
	volatile int			Internal_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
#if defined( ARCH_I386 ) || defined( ARCH_X86_64 ) || defined( ARCH_ARM )

template< typename _Ty >
BcForceInline BcAtomic< _Ty >::BcAtomic()
{
	Internal_ = 0;
}

template< typename _Ty >
BcForceInline BcAtomic< _Ty >::BcAtomic( const BcAtomic< _Ty >& Value )
{
	Internal_ = Value.Internal_;
}

template< typename _Ty >
BcForceInline BcAtomic< _Ty >::BcAtomic( _Ty Value )
{
	Internal_ = (int)Value;
}

template< typename _Ty >
BcForceInline _Ty BcAtomic< _Ty >::operator = ( const BcAtomic& Value )
{
	register int Old = __sync_val_compare_and_swap( &Internal_, Internal_, Value.Internal_ );
	return (_Ty)( Old );
}

template< typename _Ty >
BcForceInline _Ty BcAtomic< _Ty >::operator = ( _Ty Value )
{
	register int Old = __sync_val_compare_and_swap( &Internal_, Internal_, Value );
	return (_Ty)( Old );
}

template< typename _Ty >
BcForceInline BcAtomic< _Ty >::operator _Ty() const
{
	return (_Ty)( Internal_ );
}

template< typename _Ty >
BcForceInline _Ty BcAtomic< _Ty >::operator ++ ()
{
	register int Value = __sync_add_and_fetch( &Internal_, 1 );
	return (_Ty)( Value );
}

template< typename _Ty >
BcForceInline _Ty BcAtomic< _Ty >::operator -- ()
{
	register int Value = __sync_sub_and_fetch( &Internal_, 1 );
	return (_Ty)( Value );
}

template< typename _Ty >
BcForceInline _Ty BcAtomic< _Ty >::operator ++ ( int )
{
	register int Value = __sync_fetch_and_add( &Internal_, 1 );
	return (_Ty)( Value );
}

template< typename _Ty >
BcForceInline _Ty BcAtomic< _Ty >::operator -- ( int )
{
	register int Value = __sync_fetch_and_sub( &Internal_, 1 );
	return (_Ty)( Value );
}

template< typename _Ty >
BcForceInline _Ty BcAtomic< _Ty >::exchange( _Ty Value )
{
	register int Old = __sync_val_compare_and_swap( &Internal_, Internal_, Value );
	return (_Ty)( Old );
}

template< typename _Ty >
BcForceInline _Ty BcAtomic< _Ty >::compareExchange( _Ty Exchange, _Ty Comparand )
{
	register int Old = __sync_val_compare_and_swap( &Internal_, Comparand, Exchange );
	return (_Ty)( Old );
}

#else
#error Architecture not supported!
#endif
//////////////////////////////////////////////////////////////////////////
// Typedefs
typedef BcAtomic< BcU8 >			BcAtomicU8;
typedef BcAtomic< BcU16 >			BcAtomicU16;
typedef BcAtomic< BcU32 >			BcAtomicU32;
typedef BcAtomic< BcS8 >			BcAtomicS8;
typedef BcAtomic< BcS16 >			BcAtomicS16;
typedef BcAtomic< BcS32 >			BcAtomicS32;
typedef BcAtomic< BcBool >			BcAtomicBool;

#endif
