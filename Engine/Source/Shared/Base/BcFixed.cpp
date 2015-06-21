/**************************************************************************
*
* File:		BcFixed.cpp
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*		Fixed point.
*
*
*
*
**************************************************************************/

#include "Base/BcFixed.h"
#include "Base/BcMath.h"
#include "Base/BcDebug.h"

//////////////////////////////////////////////////////////////////////////
// BcFixed unit test
void BcFixed_UnitTest()
{
#define FIXED_TEST( _Ty, _Precision, _A, _Op, _B, _Expected, _Msg )					\
	{																				\
		typedef BcFixed< _Ty, _Precision > BcFixedDef;								\
		PSY_LOG( "Result: %f, Expected: %f Tolerance: %f\n", ( BcFixedDef( _A ) _Op BcFixedDef( _B ) ).asReal(), BcFixedDef( _Expected ).asReal(), BcFixedDef::STEP() ); \
		BcUnitTestMsg( ( BcAbs( ( BcFixedDef( _A ) _Op BcFixedDef( _B ) ).asReal() - BcFixedDef( _Expected ).asReal() ) <= BcFixedDef::STEP() ), _Msg ); }

#define FIXED_TEST_SET( _Ty, _Precision ) \
	FIXED_TEST( _Ty, _Precision, 1.0f, +, 1.0f, 2.0f, "Addition Test" );			\
	FIXED_TEST( _Ty, _Precision, 2.0f, +, 1.0f, 3.0f, "Addition Test" );			\
	FIXED_TEST( _Ty, _Precision, 3.0f, +, 2.0f, 5.0f, "Addition Test" );			\
	FIXED_TEST( _Ty, _Precision, 4.0f, +, 2.0f, 6.0f, "Addition Test" );			\
	FIXED_TEST( _Ty, _Precision, 1.0f, -, 1.0f, 0.0f, "Subtraction Test" );			\
	FIXED_TEST( _Ty, _Precision, 2.0f, -, 1.0f, 1.0f, "Subtraction Test" );			\
	FIXED_TEST( _Ty, _Precision, 3.0f, -, 2.0f, 1.0f, "Subtraction Test" );			\
	FIXED_TEST( _Ty, _Precision, 4.0f, -, 2.0f, 2.0f, "Subtraction Test" );			\
	FIXED_TEST( _Ty, _Precision, 1.0f, *, 1.0f, 1.0f, "Multiplication Test" );		\
	FIXED_TEST( _Ty, _Precision, 2.0f, *, 1.0f, 2.0f, "Multiplication Test" );		\
	FIXED_TEST( _Ty, _Precision, 3.0f, *, 2.0f, 6.0f, "Multiplication Test" );		\
	FIXED_TEST( _Ty, _Precision, 4.0f, *, 2.0f, 8.0f, "Multiplication Test" );		\
	FIXED_TEST( _Ty, _Precision, 1.0f, /, 1.0f, 1.0f, "Division Test" );			\
	FIXED_TEST( _Ty, _Precision, 2.0f, /, 1.0f, 2.0f, "Division Test" );			\
	FIXED_TEST( _Ty, _Precision, 3.0f, /, 2.0f, 1.5f, "Division Test" );			\
	FIXED_TEST( _Ty, _Precision, 4.0f, /, 2.0f, 2.0f, "Division Test" ); 

	FIXED_TEST_SET( BcS8, 1 );
	FIXED_TEST_SET( BcS8, 2 );
	FIXED_TEST_SET( BcS8, 4 );

	FIXED_TEST_SET( BcS16, 2 ); // Breaks on 32-bit arm. Alignment problem?
	FIXED_TEST_SET( BcS16, 4 ); // Breaks on 32-bit arm. Alignment problem?
	FIXED_TEST_SET( BcS16, 8 ); // Breaks on 32-bit arm. Alignment problem?

	FIXED_TEST_SET( BcS32, 4 );
	FIXED_TEST_SET( BcS32, 8 );
	FIXED_TEST_SET( BcS32, 16 );

	// NOTE: Never go above the half way mark with precision!
}
