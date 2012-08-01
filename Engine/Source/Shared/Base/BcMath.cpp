/**************************************************************************
*
* File:		BcMath.cpp
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*		Common maths stuff.
*
*
*
*
**************************************************************************/

#include "BcMath.h"
#include "BcFixed.h"

//////////////////////////////////////////////////////////////////////////
// BcSqrt
BcReal BcSqrt( BcReal v )
{
#if PLATFORM_WINDOWS && COMPILER_MSVC && ARCH_I386
	__asm fld [v]
	__asm fsqrt
	__asm fstp [v]
	return v;
#elif PLATFORM_LINUX
	asm(
		"fld %1\n"
		"fsqrt\n"
		"fstp %0\n"
	: "=g"( v )
	: "g"( v )
	);
	return v;
#else
	return sqrtf( v );
#endif
}

//////////////////////////////////////////////////////////////////////////
// BcSqrtFixed
BcU64 BcSqrtFixed( BcU64 FixedValue, BcU64 Precision )
{
	const BcU64 DoublePrecision = Precision << 1;
	const BcU64 Half = 2 << Precision;
	register BcU64 V = FixedValue;
	register BcU64 X =  V >> 1;
	register BcU64 A = 0;
	
	if( V > 0 && X != 0 )
	{
		do
		{
			A = ( ( V << DoublePrecision ) / X ) >> Precision;
			X = ( ( ( X + A ) << DoublePrecision ) / Half ) >> Precision;
		}
		while( ( X * X ) > ( V << Precision ) );
		
		return X;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// BcSin
BcReal BcSin( BcReal r )
{
#if PLATFORM_WINDOWS && COMPILER_MSVC && ARCH_I386
	__asm fld [r]
	__asm fsin
	__asm fstp [r]
	return r;
#elif PLATFORM_LINUX
	asm(
		"fld %1\n"
		"fsin\n"
		"fstp %0\n"
	: "=g"( r )
	: "g"( r )
	);
	return r;
#else
	return sinf( r );
#endif
}

//////////////////////////////////////////////////////////////////////////
// BcCos
BcReal BcCos( BcReal r )
{
#if PLATFORM_WINDOWS && COMPILER_MSVC && ARCH_I386
	__asm fld [r]
	__asm fcos
	__asm fstp [r]
	return r;
#elif PLATFORM_LINUX
	asm(
		"fld %1\n"
		"fcos\n"
		"fstp %0\n"
	: "=g"( r )
	: "g"( r )
	);
	return r;
#else
	return cosf( r );
#endif
}

//////////////////////////////////////////////////////////////////////////
// BcFloor
BcReal BcFloor( BcReal T )
{
	return floorf( T );
}

//////////////////////////////////////////////////////////////////////////
// BcCeil
BcReal BcCeil( BcReal T )
{
	return ceilf( T );
}

