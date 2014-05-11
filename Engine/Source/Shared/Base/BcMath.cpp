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
BcF32 BcSqrt( BcF32 v )
{
#if PLATFORM_WINDOWS && COMPILER_MSVC && ARCH_X86
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
BcF32 BcSin( BcF32 r )
{
#if PLATFORM_WINDOWS && COMPILER_MSVC && ARCH_X86
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
BcF32 BcCos( BcF32 r )
{
#if PLATFORM_WINDOWS && COMPILER_MSVC && ARCH_X86
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
BcF32 BcTan( BcF32 r )
{
	return tanf( r );
}

//////////////////////////////////////////////////////////////////////////
// BcAsin
BcF32 BcAsin( BcF32 r )
{
	return asinf( r );
}


//////////////////////////////////////////////////////////////////////////
// BcAcos
BcF32 BcAcos( BcF32 r )
{
	return acosf( r );
}

//////////////////////////////////////////////////////////////////////////
// BcFloor
BcF32 BcFloor( BcF32 T )
{
	return floorf( T );
}

//////////////////////////////////////////////////////////////////////////
// BcCeil
BcF32 BcCeil( BcF32 T )
{
	return ceilf( T );
}

//////////////////////////////////////////////////////////////////////////
// BcAtan2
BcF32 BcAtan2( BcF32 Y, BcF32 X )
{
#if PLATFORM_WINDOWS && COMPILER_MSVC && ARCH_X86
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
// BcLerp
BcF32 BcLerp( BcF32 A, BcF32 B, BcF32 T )
{
	return ( A * ( 1.0f - T ) ) + ( B * T );
}

//////////////////////////////////////////////////////////////////////////
// BcSigmoid
BcF32 BcSigmoid( BcF32 T )
{
	return ( 1.0f / ( 1.0f + BcExp( -T ) ) );
}

//////////////////////////////////////////////////////////////////////////
// BcSigmoidRange
BcF32 BcSigmoidRange( BcF32 T, BcF32 Min, BcF32 Max, BcF32 Scale, BcF32 Bias )
{
	BcF32 S = BcSigmoid( ( T * Scale ) + Bias );

	return ( Min + ( ( Max - Min ) * S ) );
}

//////////////////////////////////////////////////////////////////////////
// BcPot
BcBool BcPot( BcU32 T )
{
	return ( ( T & ( T - 1 ) ) == 0 ) || ( T == 1 );
}

BcBool BcPot( BcU64 T )
{
	return ( ( T & ( T - 1 ) ) == 0 ) || ( T == 1 );
}

//////////////////////////////////////////////////////////////////////////
// BcPotRoundUp
BcU32 BcPotRoundUp( BcU32 Value, BcU32 RoundUpTo )
{
	BcAssert( BcPot( RoundUpTo ) == BcTrue );
	return ( Value + ( RoundUpTo - 1 ) ) & ~( ( RoundUpTo - 1 ) );
}

BcU64 BcPotRoundUp( BcU64 Value, BcU64 RoundUpTo )
{
	BcAssert( BcPot( RoundUpTo ) == BcTrue );
	return ( Value + ( RoundUpTo - 1 ) ) & ~( ( RoundUpTo - 1 ) );
}

//////////////////////////////////////////////////////////////////////////
// BcPotNext
BcU32 BcPotNext( BcU32 Value )
{
	--Value;
	Value |= Value >> 1;
	Value |= Value >> 2;
	Value |= Value >> 4;
	Value |= Value >> 8;
	Value |= Value >> 16;
	return ++Value;
}

BcU64 BcPotNext( BcU64 Value )
{
	--Value;
	Value |= Value >> 1;
	Value |= Value >> 2;
	Value |= Value >> 4;
	Value |= Value >> 8;
	Value |= Value >> 16;
	Value |= Value >> 32;
	return ++Value;
}

//////////////////////////////////////////////////////////////////////////
// BcCalcAlignment
BcU32 BcCalcAlignment( BcU32 Value, BcU32 Alignment )
{
	BcAssert( BcPot( Alignment ) );
	return ( Value + ( Alignment - 1 ) ) & ~( Alignment - 1 );
}

BcU64 BcCalcAlignment( BcU64 Value, BcU32 Alignment )
{
	BcAssert( BcPot( Alignment ) );
	return ( Value + BcU64( Alignment - 1 ) ) & ~( BcU64( Alignment - 1 ) );
}

BcU64 BcCalcAlignment( BcU64 Value, BcU64 Alignment )
{
	BcAssert( BcPot( Alignment ) );
	return ( Value + ( Alignment - 1 ) ) & ~( Alignment - 1 );
}

//////////////////////////////////////////////////////////////////////////
// BcBitsSet
BcU32 BcBitsSet( BcU32 Value )
{
	Value = ( Value & 0x55555555U ) + ( ( Value & 0xAAAAAAAAU ) >> 1 );
    Value = ( Value & 0x33333333U ) + ( ( Value & 0xCCCCCCCCU ) >> 2 );
    Value = ( Value & 0x0F0F0F0FU ) + ( ( Value & 0xF0F0F0F0U ) >> 4 );
    Value = ( Value & 0x00FF00FFU ) + ( ( Value & 0xFF00FF00U ) >> 8 );
    return  ( Value & 0x0000FFFFU ) + ( ( Value & 0xFFFF0000U ) >> 16 );
}

BcU32 BcBitsSet( BcU64 Value )
{
	return BcBitsSet( static_cast< BcU32 >( Value & 0xffffffff ) ) + 
	       BcBitsSet( static_cast< BcU32 >( Value >> 32 ) );
}

//////////////////////////////////////////////////////////////////////////
// BcSmoothStep
BcF32 BcSmoothStep( BcF32 T )
{
	return T * T * ( 3 - 2 * T );
}

//////////////////////////////////////////////////////////////////////////
// BcSquaredKeepSign
BcF32 BcSquaredKeepSign( BcF32 T )
{
	return ( T * T ) * ( T > 0.0f ? 1.0f : -1.0f );
}

//////////////////////////////////////////////////////////////////////////
// BcCheckFloat
#define BC_FLOAT_SIGN_MASK 0x80000000
#define BC_FLOAT_EXP_MASK 0x7F800000
#define BC_FLOAT_FRAC_MASK 0x007FFFFF
#define BC_FLOAT_SNAN_MASK 0x00400000

BcBool BcCheckFloat( BcF32 T )
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
