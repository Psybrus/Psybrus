/**************************************************************************
*
* File:		cMath.h
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*		Common maths stuff.
*
*
*
*
**************************************************************************/

#ifndef __BCMATH_H__
#define __BCMATH_H__

#include "Base/BcDebug.h"
#include "Base/BcTypes.h"

// Streflop include.
#ifdef STREFLOP_X87
#include "SMath.h"
using namespace streflop;
#else
#include <math.h>
#endif

//////////////////////////////////////////////////////////////////////////
// Some undefining from math.h
#ifdef min
#undef min
#define min min
#endif

#ifdef max
#undef max
#define max max
#endif

//////////////////////////////////////////////////////////////////////////
// Epsilons
#define BcVecEpsilon					( 0.001f )

//////////////////////////////////////////////////////////////////////////
// Useful defines
#define BcPI							(3.14159265358979310f)
#define BcPIMUL2						(6.28318530717958620f)
#define BcPIMUL4						(12.5663706143591720f)
#define BcPIDIV2						(1.57079632679489660f)
#define BcPIDIV4						(0.78539816339744828f)


//////////////////////////////////////////////////////////////////////////
// BcSqrt
inline BcReal BcSqrt( BcReal v )
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
// BcSin
inline BcReal BcSin( BcReal r )
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
inline BcReal BcCos( BcReal r )
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
// BcTan
inline BcReal BcTan( BcReal r )
{
	return tanf( r );
}

//////////////////////////////////////////////////////////////////////////
// BcAsin
inline BcReal BcAsin( BcReal r )
{
	return asinf( r );
}


//////////////////////////////////////////////////////////////////////////
// BcAcos
inline BcReal BcAcos( BcReal r )
{
	return acosf( r );
}

//////////////////////////////////////////////////////////////////////////
// BcAtan2
inline BcReal BcAtan2( BcReal Y, BcReal X )
{
#if PLATFORM_WINDOWS && COMPILER_MSVC && ARCH_I386
	__asm fld [Y]
	__asm fld [X]
	__asm fpatan
	__asm fstp [Y]
	return Y;
#elif PLATFORM_LINUX
	asm(
		"fld %1\n"
		"fld %2\n"
		"fpatan\n"
		"fstp %0\n"
	: "=g"( Y )
	: "g"( Y ), "g"( X )
	);
	return Y;
#else
	return atan2( Y, X );
#endif
}

//////////////////////////////////////////////////////////////////////////
// BcExp
inline BcReal BcExp( BcReal T )
{
	return expf( T );
}

//////////////////////////////////////////////////////////////////////////
// BcAbs
template< typename _Ty >
inline _Ty BcAbs( _Ty T )
{
	return T < _Ty( 0 ) ? -T : T;
}

//////////////////////////////////////////////////////////////////////////
// BcFloor
inline BcReal BcFloor( BcReal T )
{
	return floorf( T );
}

//////////////////////////////////////////////////////////////////////////
// BcCeil
inline BcReal BcCeil( BcReal T )
{
	return ceilf( T );
}

//////////////////////////////////////////////////////////////////////////
// BcRound
inline BcReal BcRound( BcReal T )
{
	return BcFloor( T + 0.5f );
}

//////////////////////////////////////////////////////////////////////////
// BcClamp
template< typename _Ty, typename _Uy, typename _Vy >
inline _Ty BcClamp( _Ty Value, _Uy Min, _Vy Max )
{
	if( Value > (_Ty)Max )
	{
		return (_Ty)Max;
	}
	else if ( Value < (_Ty)Min )
	{
		return (_Ty)Min;
	}

	return Value;
}

//////////////////////////////////////////////////////////////////////////
// BcMin
template< typename _Ty >
inline _Ty BcMin( _Ty A, _Ty B )
{
	return ( A < B ) ? A : B;
}

//////////////////////////////////////////////////////////////////////////
// BcMax
template< typename _Ty >
inline _Ty BcMax( _Ty A, _Ty B )
{
	return ( A > B ) ? A : B;
}

//////////////////////////////////////////////////////////////////////////
// BcLerp
inline BcReal BcLerp( BcReal A, BcReal B, BcReal T )
{
	return ( A * ( 1.0f - T ) ) + ( B * T );
}

//////////////////////////////////////////////////////////////////////////
// BcSigmoid
inline BcReal BcSigmoid( BcReal T )
{
	return ( 1.0f / ( 1.0f + BcExp( -T ) ) );
}

//////////////////////////////////////////////////////////////////////////
// BcSigmoidRange
inline BcReal BcSigmoidRange( BcReal T, BcReal Min, BcReal Max, BcReal Scale, BcReal Bias )
{
	BcReal S = BcSigmoid( ( T * Scale ) + Bias );

	return ( Min + ( ( Max - Min ) * S ) );
}

//////////////////////////////////////////////////////////////////////////
// BcPot
inline BcBool BcPot( BcU32 T )
{
	return ( ( T & ( T - 1 ) ) == 0 ) || ( T == 1 );
}

//////////////////////////////////////////////////////////////////////////
// BcPotRoundUp
inline BcU32 BcPotRoundUp( BcU32 Value, BcU32 RoundUpTo )
{
	BcAssert( BcPot( RoundUpTo ) == BcTrue );
	return ( Value + ( RoundUpTo - 1 ) ) & ~( ( RoundUpTo - 1 ) );
}

//////////////////////////////////////////////////////////////////////////
// BcPotNext
inline BcU32 BcPotNext( BcU32 Value )
{
	--Value;
	Value |= Value >> 1;
	Value |= Value >> 2;
	Value |= Value >> 4;
	Value |= Value >> 8;
	Value |= Value >> 16;
	return ++Value;
}

//////////////////////////////////////////////////////////////////////////
// BcBitsSet
inline BcU32 BcBitsSet( BcU32 Value )
{
	Value = ( Value & 0x55555555U ) + ( ( Value & 0xAAAAAAAAU ) >> 1 );
    Value = ( Value & 0x33333333U ) + ( ( Value & 0xCCCCCCCCU ) >> 2 );
    Value = ( Value & 0x0F0F0F0FU ) + ( ( Value & 0xF0F0F0F0U ) >> 4 );
    Value = ( Value & 0x00FF00FFU ) + ( ( Value & 0xFF00FF00U ) >> 8 );
    return  ( Value & 0x0000FFFFU ) + ( ( Value & 0xFFFF0000U ) >> 16 );
}

//////////////////////////////////////////////////////////////////////////
// BcSmoothStep
inline BcReal BcSmoothStep( BcReal T )
{
	return T * T * ( 3 - 2 * T );
}

//////////////////////////////////////////////////////////////////////////
// BcSquaredKeepSign
inline BcReal BcSquaredKeepSign( BcReal T )
{
	return ( T * T ) * ( T > 0.0f ? 1.0f : -1.0f );
}

//////////////////////////////////////////////////////////////////////////
// BcCheckFloat
#define BC_FLOAT_SIGN_MASK 0x80000000
#define BC_FLOAT_EXP_MASK 0x7F800000
#define BC_FLOAT_FRAC_MASK 0x007FFFFF
#define BC_FLOAT_SNAN_MASK 0x00400000

inline BcBool BcCheckFloat( BcF32 T )
{
	BcF32 Copy = T;
	BcU32 IntFloat = *( ( BcU32* ) &(Copy) );
	BcU32 Exp = IntFloat & BC_FLOAT_EXP_MASK;
	BcU32 Frac = IntFloat & BC_FLOAT_FRAC_MASK;
	if ( ( Exp == 0 ) && ( Frac != 0 ) ) return BcFalse;
	if ( Exp == BC_FLOAT_EXP_MASK ) return BcFalse;
	if ( ( Exp == BC_FLOAT_EXP_MASK ) && ( ( Frac & BC_FLOAT_SNAN_MASK ) == 0 ) ) return BcFalse;
	return BcTrue;
}

#endif
