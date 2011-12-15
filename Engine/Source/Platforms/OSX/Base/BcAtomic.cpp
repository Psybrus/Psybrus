/**************************************************************************
*
* File:		BcAtomic.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
*			
* Description:
*		Atomic data structures
*		
*		
* 
**************************************************************************/

#include "BcAtomic.h"
#include "BcDebug.h"

//////////////////////////////////////////////////////////////////////////
// BcAtomic unit test
void BcAtomic_UnitTest()
{
#define ATOMIC_TEST( _test, _msg )											\
	{																		\
		BcBool TestResult = ( _test );										\
		BcAssertMsg( _test, _msg ":" #_test );								\
		BcUnusedVar( TestResult );											\
	}

#define ATOMIC_TEST_EXCHANGE( _T, _ia, _ib )								\
	{																		\
		BcAtomic< _T > AtomicA;												\
		_T B;																\
		AtomicA = (_T)_ia;													\
		B = (_T)_ib;														\
		B = AtomicA.exchange( B );											\
		ATOMIC_TEST( AtomicA == (_T)_ib && B == (_T)_ia, "Exchange" );		\
	}

#define ATOMIC_TEST_PREDEC( _T, _ia )										\
	{																		\
		BcAtomic< _T > AtomicA = (_T)_ia;									\
		_T AtomicResult = --AtomicA;										\
		_T A = (_T)_ia;														\
		_T Result = --A;													\
		ATOMIC_TEST( AtomicResult == Result, "Predecrement" );				\
	}

#define ATOMIC_TEST_PREINC( _T, _ia )										\
	{																		\
		BcAtomic< _T > AtomicA = (_T)_ia;									\
		_T AtomicResult = ++AtomicA;										\
		_T A = (_T)_ia;														\
		_T Result = ++A;													\
		ATOMIC_TEST( AtomicResult == Result, "Preincrement" );				\
	}

#define ATOMIC_TEST_POSTDEC( _T, _ia )										\
	{																		\
		BcAtomic< _T > AtomicA = (_T)_ia;									\
		_T AtomicResult = AtomicA--;										\
		_T A = (_T)_ia;														\
		_T Result = A--;													\
		ATOMIC_TEST( AtomicResult == Result, "Postdecrement" );				\
	}

#define ATOMIC_TEST_POSTINC( _T, _ia )										\
	{																		\
		BcAtomic< _T > AtomicA = (_T)_ia;									\
		_T AtomicResult = AtomicA++;										\
		_T A = (_T)_ia;														\
		_T Result = A++;													\
		ATOMIC_TEST( AtomicResult == Result, "Postincrement" );				\
	}

	// List of exchange tests for every type to use.
#define ATOMIC_TEST_EXCHANGE_TYPE( _T )										\
	ATOMIC_TEST_EXCHANGE( _T, 1, 0 );										\
	ATOMIC_TEST_EXCHANGE( _T, 0xff, 0 );									\
	ATOMIC_TEST_EXCHANGE( _T, 0xff, 0xfe );									\
	ATOMIC_TEST_EXCHANGE( _T, 0x100, 0 );									\
	ATOMIC_TEST_EXCHANGE( _T, 0x7f, 0x80 );									\
	ATOMIC_TEST_EXCHANGE( _T, 0xffff, 0 );									\
	ATOMIC_TEST_EXCHANGE( _T, 0xffff, 0xfffe );								\
	ATOMIC_TEST_EXCHANGE( _T, 0x10000, 0 );									\
	ATOMIC_TEST_EXCHANGE( _T, 0x7fff, 0x8000 );								\
	ATOMIC_TEST_EXCHANGE( _T, 0xffffffff, 0 );								\
	ATOMIC_TEST_EXCHANGE( _T, 0xffffffff, 0xfffffffe );						\
	ATOMIC_TEST_EXCHANGE( _T, 0x7fffffff, 0x80000000 );						
//	ATOMIC_TEST_EXCHANGE( _T, 0x100000000, 0 );								

	// List of types for all increment/decrements to do.
#define ATOMIC_TEST_BASIC_ARITHMETIC( _T, _Op )								\
	_Op( _T, 0 );															\
	_Op( _T, 1 );															\
	_Op( _T, 0xff );														\
	_Op( _T, 0xfe );														\
	_Op( _T, 0x7f );														\
	_Op( _T, 0x80 );														\
	_Op( _T, 0xffff );														\
	_Op( _T, 0xfffe );														\
	_Op( _T, 0x7fff );														\
	_Op( _T, 0x8000 );														\
	_Op( _T, 0xffffffff );													\
	_Op( _T, 0xfffffffe );													\
	_Op( _T, 0x7fffffff );													\
	_Op( _T, 0x80000000 );													
//	_Op( _T, 0x100000000 );									

	// Arithmetic op types
#define ATOMIC_TEST_ARITHMETIC_TYPE( _Ty )									\
	ATOMIC_TEST_BASIC_ARITHMETIC( _Ty, ATOMIC_TEST_PREDEC );				\
	ATOMIC_TEST_BASIC_ARITHMETIC( _Ty, ATOMIC_TEST_POSTDEC );				\
	ATOMIC_TEST_BASIC_ARITHMETIC( _Ty, ATOMIC_TEST_PREINC );				\
	ATOMIC_TEST_BASIC_ARITHMETIC( _Ty, ATOMIC_TEST_POSTINC );
	
	// Do arithmetic tests.
	ATOMIC_TEST_ARITHMETIC_TYPE( BcU8 );
	ATOMIC_TEST_ARITHMETIC_TYPE( BcS8 );
	ATOMIC_TEST_ARITHMETIC_TYPE( BcU16 );
	ATOMIC_TEST_ARITHMETIC_TYPE( BcS16 );
	ATOMIC_TEST_ARITHMETIC_TYPE( BcU32 );
	ATOMIC_TEST_ARITHMETIC_TYPE( BcS32 );
	ATOMIC_TEST_ARITHMETIC_TYPE( BcChar );
	
	// Does not perform arithmetic on pointer types yet.
	//ATOMIC_TEST_ARITHMETIC_TYPE( BcU8* );			
	//ATOMIC_TEST_ARITHMETIC_TYPE( BcU16* );
	//ATOMIC_TEST_ARITHMETIC_TYPE( BcU32* );
	
	// Do exchange tests.
	ATOMIC_TEST_EXCHANGE_TYPE( BcU8 );
	ATOMIC_TEST_EXCHANGE_TYPE( BcS8 );
	ATOMIC_TEST_EXCHANGE_TYPE( BcU16 );
	ATOMIC_TEST_EXCHANGE_TYPE( BcS16 );
	ATOMIC_TEST_EXCHANGE_TYPE( BcU32 );
	ATOMIC_TEST_EXCHANGE_TYPE( BcS32 );
	ATOMIC_TEST_EXCHANGE_TYPE( BcChar );
	ATOMIC_TEST_EXCHANGE_TYPE( BcU8* );
	ATOMIC_TEST_EXCHANGE_TYPE( BcU16* );
	ATOMIC_TEST_EXCHANGE_TYPE( BcU32* );
}
