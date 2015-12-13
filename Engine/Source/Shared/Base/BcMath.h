/**************************************************************************
*
* File:		BcMath.h
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*		Common maths stuff.
*
*
*
*
*************************************************************************/

#ifndef __BCMATH_H__
#define __BCMATH_H__

#include "Base/BcDebug.h"
#include "Base/BcTypes.h"

#include <cmath>

// cmath will define this. We say no.
#undef DOMAIN
#define DOMAIN DOMAIN

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
// BcSqrtFixed
BcU64 BcSqrtFixed( BcU64 FixedValue, BcU64 Precision );

//////////////////////////////////////////////////////////////////////////
// BcSqrt
BcF32 BcSqrt( BcF32 v );

//////////////////////////////////////////////////////////////////////////
// BcSin
BcF32 BcSin( BcF32 r );

//////////////////////////////////////////////////////////////////////////
// BcCos
BcF32 BcCos( BcF32 r );

//////////////////////////////////////////////////////////////////////////
// BcTan
BcF32 BcTan( BcF32 r );

//////////////////////////////////////////////////////////////////////////
// BcAsin
BcF32 BcAsin( BcF32 r );

//////////////////////////////////////////////////////////////////////////
// BcAcos
BcF32 BcAcos( BcF32 r );

//////////////////////////////////////////////////////////////////////////
// BcAtan2
BcF32 BcAtan2( BcF32 Y, BcF32 X );

//////////////////////////////////////////////////////////////////////////
// BcExp
inline BcF32 BcExp( BcF32 T )
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
BcF32 BcFloor( BcF32 T );

//////////////////////////////////////////////////////////////////////////
// BcCeil
BcF32 BcCeil( BcF32 T );

//////////////////////////////////////////////////////////////////////////
// BcRound
inline BcF32 BcRound( BcF32 T )
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
template< typename _Ty, typename _Uy >
inline _Ty BcMin( _Ty A, _Uy B )
{
	return ( A < (_Ty)B ) ? A : (_Ty)B;
}

//////////////////////////////////////////////////////////////////////////
// BcMax
template< typename _Ty, typename _Uy >
inline _Ty BcMax( _Ty A, _Uy B )
{
	return ( A > (_Ty)B ) ? A : (_Ty)B;
}

//////////////////////////////////////////////////////////////////////////
// BcLerp
BcF32 BcLerp( BcF32 A, BcF32 B, BcF32 T );

//////////////////////////////////////////////////////////////////////////
// BcSigmoid
BcF32 BcSigmoid( BcF32 T );

//////////////////////////////////////////////////////////////////////////
// BcSigmoidRange
BcF32 BcSigmoidRange( BcF32 T, BcF32 Min, BcF32 Max, BcF32 Scale, BcF32 Bias );

//////////////////////////////////////////////////////////////////////////
// BcPot
template< typename _Ty >
inline BcBool BcPot( _Ty T )
{
	return ( ( T & ( T - 1 ) ) == 0 ) || ( T == 1 );
}

//////////////////////////////////////////////////////////////////////////
// BcPotRoundUp 
template< typename _Ty, typename _Uy >
inline _Ty BcPotRoundUp( _Ty Value, _Uy RoundUpTo )
{
	BcAssert( BcPot( RoundUpTo ) == BcTrue );
	return ( Value + ( static_cast< _Ty >( RoundUpTo ) - 1 ) ) & ~( ( static_cast< _Ty >( RoundUpTo ) - 1 ) );
}

//////////////////////////////////////////////////////////////////////////
// BcPotNext
BcU32 BcPotNext( BcU32 Value );
BcU64 BcPotNext( BcU64 Value );

//////////////////////////////////////////////////////////////////////////
// BcCalcAlignment
BcU32 BcCalcAlignment( BcU32 Value, BcU32 Alignment );
BcU64 BcCalcAlignment( BcU64 Value, BcU32 Alignment );
BcU64 BcCalcAlignment( BcU64 Value, BcU64 Alignment );

//////////////////////////////////////////////////////////////////////////
// BcBitsSet
BcU32 BcBitsSet( BcU32 Value );
BcU32 BcBitsSet( BcU64 Value );

//////////////////////////////////////////////////////////////////////////
// BcSmoothStep
BcF32 BcSmoothStep( BcF32 T );

//////////////////////////////////////////////////////////////////////////
// BcSquaredKeepSign
BcF32 BcSquaredKeepSign( BcF32 T );

//////////////////////////////////////////////////////////////////////////
// BcCheckFloat
BcBool BcCheckFloat( BcF32 T );

#endif
