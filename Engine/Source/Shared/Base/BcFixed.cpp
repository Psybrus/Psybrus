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
// Unit tests.
#if !PSY_PRODUCTION
#include <catch.hpp>

#define FIXED_TEST( _Ty, _Precision, _A, _Op, _B, _Expected )						\
	{																				\
		typedef BcFixed< _Ty, _Precision > BcFixedDef;								\
		REQUIRE( ( BcAbs( ( BcFixedDef( _A ) _Op BcFixedDef( _B ) ).asReal() - BcFixedDef( _Expected ).asReal() ) <= BcFixedDef::STEP() ) ); }

TEST_CASE( "BcFixed-Addition")
{
#define FIXED_TEST_SET( _Ty, _Precision ) \
	FIXED_TEST( _Ty, _Precision, 1.0f, +, 1.0f, 2.0f );			\
	FIXED_TEST( _Ty, _Precision, 2.0f, +, 1.0f, 3.0f );			\
	FIXED_TEST( _Ty, _Precision, 3.0f, +, 2.0f, 5.0f );			\
	FIXED_TEST( _Ty, _Precision, 4.0f, +, 2.0f, 6.0f );

	FIXED_TEST_SET( BcS8, 1 );
	FIXED_TEST_SET( BcS8, 2 );
	FIXED_TEST_SET( BcS8, 4 );


#if !ARCH_ARM // Breaks on 32-bit arm. Alignment problem?
	FIXED_TEST_SET( BcS16, 2 );
	FIXED_TEST_SET( BcS16, 4 );
	FIXED_TEST_SET( BcS16, 8 );
#endif

	FIXED_TEST_SET( BcS32, 4 );
	FIXED_TEST_SET( BcS32, 8 );
	FIXED_TEST_SET( BcS32, 16 );

#undef FIXED_TEST_SET
}

TEST_CASE( "BcFixed-Subtraction")
{
#define FIXED_TEST_SET( _Ty, _Precision ) \
	FIXED_TEST( _Ty, _Precision, 1.0f, -, 1.0f, 0.0f );			\
	FIXED_TEST( _Ty, _Precision, 2.0f, -, 1.0f, 1.0f );			\
	FIXED_TEST( _Ty, _Precision, 3.0f, -, 2.0f, 1.0f );			\
	FIXED_TEST( _Ty, _Precision, 4.0f, -, 2.0f, 2.0f );

	FIXED_TEST_SET( BcS8, 1 );
	FIXED_TEST_SET( BcS8, 2 );
	FIXED_TEST_SET( BcS8, 4 );


#if !ARCH_ARM // Breaks on 32-bit arm. Alignment problem?
	FIXED_TEST_SET( BcS16, 2 );
	FIXED_TEST_SET( BcS16, 4 );
	FIXED_TEST_SET( BcS16, 8 );
#endif

	FIXED_TEST_SET( BcS32, 4 );
	FIXED_TEST_SET( BcS32, 8 );
	FIXED_TEST_SET( BcS32, 16 );

#undef FIXED_TEST_SET
}

TEST_CASE( "BcFixed-Multiplication")
{
#define FIXED_TEST_SET( _Ty, _Precision ) \
	FIXED_TEST( _Ty, _Precision, 1.0f, *, 1.0f, 1.0f );		\
	FIXED_TEST( _Ty, _Precision, 2.0f, *, 1.0f, 2.0f );		\
	FIXED_TEST( _Ty, _Precision, 3.0f, *, 2.0f, 6.0f );		\
	FIXED_TEST( _Ty, _Precision, 4.0f, *, 2.0f, 8.0f );

	FIXED_TEST_SET( BcS8, 1 );
	FIXED_TEST_SET( BcS8, 2 );
	FIXED_TEST_SET( BcS8, 4 );


#if !ARCH_ARM // Breaks on 32-bit arm. Alignment problem?
	FIXED_TEST_SET( BcS16, 2 );
	FIXED_TEST_SET( BcS16, 4 );
	FIXED_TEST_SET( BcS16, 8 );
#endif

	FIXED_TEST_SET( BcS32, 4 );
	FIXED_TEST_SET( BcS32, 8 );
	FIXED_TEST_SET( BcS32, 16 );

#undef FIXED_TEST_SET
}

TEST_CASE( "BcFixed-Division")
{
#define FIXED_TEST_SET( _Ty, _Precision ) \
	FIXED_TEST( _Ty, _Precision, 1.0f, /, 1.0f, 1.0f );			\
	FIXED_TEST( _Ty, _Precision, 2.0f, /, 1.0f, 2.0f );			\
	FIXED_TEST( _Ty, _Precision, 3.0f, /, 2.0f, 1.5f );			\
	FIXED_TEST( _Ty, _Precision, 4.0f, /, 2.0f, 2.0f );

	FIXED_TEST_SET( BcS8, 1 );
	FIXED_TEST_SET( BcS8, 2 );
	FIXED_TEST_SET( BcS8, 4 );


#if !ARCH_ARM // Breaks on 32-bit arm. Alignment problem?
	FIXED_TEST_SET( BcS16, 2 );
	FIXED_TEST_SET( BcS16, 4 );
	FIXED_TEST_SET( BcS16, 8 );
#endif

	FIXED_TEST_SET( BcS32, 4 );
	FIXED_TEST_SET( BcS32, 8 );
	FIXED_TEST_SET( BcS32, 16 );

#undef FIXED_TEST_SET
}

#endif // !PSY_PRODUCTION
