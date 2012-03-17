/**************************************************************************
*
* File:		BcQuat.cpp
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		
*		
*		
*		
* 
**************************************************************************/

#include "BcDebug.h"
#include "BcQuat.h"

#include "BcString.h"

BcQuat::BcQuat( const BcChar* pString ):
	BcVec4d( pString )
{

}

#define SLERP_EPSILON	0.001f

void BcQuat::lerp(const BcQuat& a, const BcQuat& b, BcReal t)
{
	BcReal lK0 = 1.0f - t;
	BcReal lK1 = t;
	
	set((a.x() * lK0) + (b.x() * lK1),
	    (a.y() * lK0) + (b.y() * lK1),
	    (a.z() * lK0) + (b.z() * lK1),
	    (a.w() * lK0) + (b.w() * lK1));
}
	
void BcQuat::slerp( const BcQuat& a, const BcQuat& b, BcReal t)
{
	// The following code is based on what Dunn & Parberry do.
	BcReal lCosOmega = (a.w() * b.w()) + (a.x() * b.x()) + (a.y() * b.y()) + (a.z() * b.z());
	BcReal lK0;
	BcReal lK1;
	BcQuat a2 = a;

	if (lCosOmega < 0.0f)
	{
		a2.x( -a2.x() );
		a2.y( -a2.y() );
		a2.z( -a2.z() );
		a2.w( -a2.w() );
		lCosOmega = -lCosOmega;
	}
	
	if (lCosOmega > (1.0f - SLERP_EPSILON))
	{
		lK0 = 1.0f - t;
		lK1 = t;
	}
	else
	{
		// Do Slerp.
		BcReal lSinOmega = BcSqrt(1.0f - (lCosOmega * lCosOmega));
		BcReal lOmega = atan2f(lSinOmega, lCosOmega);
		BcReal lInvSinOmega = (1.0f / lSinOmega);

		lK0 = BcSin((1.0f - t) * lOmega) * lInvSinOmega;
		lK1 = BcSin(t * lOmega) * lInvSinOmega;
	}

	set((a2.x() * lK0) + (b.x() * lK1),
		(a2.y() * lK0) + (b.y() * lK1),
		(a2.z() * lK0) + (b.z() * lK1),
		(a2.w() * lK0) + (b.w() * lK1));	
}

//////////////////////////////////////////////////////////////////////////
// fromMatrix4d
void BcQuat::fromMatrix4d( const BcMat4d& Mat )
{
	BcReal FourWSqMinus1 = Mat[0][0] + Mat[1][1] + Mat[2][2];
	BcReal FourXSqMinus1 = Mat[0][0] - Mat[1][1] - Mat[2][2];
	BcReal FourYSqMinus1 = Mat[1][1] - Mat[0][0] - Mat[2][2];
	BcReal FourZSqMinus1 = Mat[2][2] - Mat[0][0] - Mat[1][1];

	BcU32 BiggestIndex = 0;
	BcReal FourBiggestSqMinus1 = FourWSqMinus1;
	if( FourXSqMinus1 > FourBiggestSqMinus1 )
	{
		FourBiggestSqMinus1 = FourXSqMinus1;
		BiggestIndex = 1;
	}
	if( FourYSqMinus1 > FourBiggestSqMinus1 )
	{
		FourBiggestSqMinus1 = FourYSqMinus1;
		BiggestIndex = 2;
	}
	if( FourZSqMinus1 > FourBiggestSqMinus1 )
	{
		FourBiggestSqMinus1 = FourZSqMinus1;
		BiggestIndex = 3;
	}

	BcReal BiggestVal = BcSqrt( FourBiggestSqMinus1 + 1.0f ) * 0.5f;
	BcReal Mult = 0.25f / BiggestVal;

	switch( BiggestIndex )
	{
	case 0:
		{
			w( BiggestVal );
			x( ( Mat[1][2] - Mat[2][1])  * Mult );
			y( ( Mat[2][0] - Mat[0][2])  * Mult );
			z( ( Mat[0][1] - Mat[1][0])  * Mult );
		}
		break;

	case 1:
		{
			w( BiggestVal );
			x( ( Mat[1][2] - Mat[2][1])  * Mult );
			y( ( Mat[2][0] - Mat[0][2])  * Mult );
			z( ( Mat[0][1] - Mat[1][0])  * Mult );
		}
		break;

	case 2:
		{
			y( BiggestVal );
			w( ( Mat[2][0] - Mat[1][3])  * Mult );
			x( ( Mat[0][1] + Mat[1][0])  * Mult );
			z( ( Mat[1][2] + Mat[2][1])  * Mult );
		}
		break;

	case 3:
		{
			z( BiggestVal );
			w( ( Mat[0][1] - Mat[1][0])  * Mult );
			x( ( Mat[2][0] + Mat[0][2])  * Mult );
			y( ( Mat[1][2] + Mat[2][1])  * Mult );
		}
		break;
	}
}

void BcQuat::asMatrix4d( BcMat4d& Matrix )  const
{
	// Multiply out the values and store in a variable
	// since storing in variables is quicker than 
	// multiplying floating point numbers again and again.
	// This should make this function a touch faster.
	
	// Set of w() multiplications required
	const BcReal lWX2 = 2.0f * w() * x();
	const BcReal lWY2 = 2.0f * w() * y();
	const BcReal lWZ2 = 2.0f * w() * z();

	// Set of x() multiplications required
	const BcReal lXX2 = 2.0f * x() * x();
	const BcReal lXY2 = 2.0f * x() * y();
	const BcReal lXZ2 = 2.0f * x() * z();

	// Remainder of y() multiplications
	const BcReal lYY2 = 2.0f * y() * y();
	const BcReal lYZ2 = 2.0f * y() * z();

	// Remainder of z() multiplications
	const BcReal lZZ2 = 2.0f * z() * z();
	
	Matrix[0][0] = ( 1.0f - ( lYY2 + lZZ2 ) );	
	Matrix[0][1] = ( lXY2 + lWZ2 );
	Matrix[0][2] = ( lXZ2 - lWY2 );
	Matrix[0][3] = ( 0.0f );
	
	Matrix[1][0] = ( lXY2 - lWZ2 );
	Matrix[1][1] = ( 1.0f - ( lXX2 + lZZ2 ) );
	Matrix[1][2] = ( lYZ2 + lWX2 );
	Matrix[1][3] = ( 0.0f );

	Matrix[2][0] = ( lXZ2 + lWY2 );
	Matrix[2][1] = ( lYZ2 - lWX2 );
	Matrix[2][2] = ( 1.0f - ( lXX2 + lYY2 ) );
	Matrix[2][3] = ( 0.0f );
	
	Matrix[3][0] = ( 0.0f );
	Matrix[3][1] = ( 0.0f );
	Matrix[3][2] = ( 0.0f );
	Matrix[3][3] = ( 1.0f );
}

void BcQuat::calcFromXYZ()
{
	BcReal t = 1.0f - ( x() * x() )
	                - ( y() * y() )
	                - ( z() * z() );

	if (t < 0.0f)
	{
		w( 0.0f );
	}
	else
	{
		w( -BcSqrt(t) );
	}
}

void BcQuat::fromEular( BcReal Yaw, BcReal Pitch, BcReal Roll )
{
	const BcReal Sin2Y = BcSin( Yaw / 2.0f );
	const BcReal Cos2Y = BcCos( Yaw / 2.0f );
	const BcReal Sin2P = BcSin( Pitch / 2.0f );
	const BcReal Cos2P = BcCos( Pitch / 2.0f );
	const BcReal Sin2R = BcSin( Roll / 2.0f );
	const BcReal Cos2R = BcCos( Roll / 2.0f );

	w(  ( Cos2Y * Cos2P * Cos2R ) + ( Sin2Y * Sin2P * Sin2R ) );
	x( -( Cos2Y * Sin2P * Cos2R ) - ( Sin2Y * Cos2P * Sin2R ) );
	y(  ( Cos2Y * Sin2P * Sin2R ) - ( Sin2Y * Cos2P * Cos2R ) );
	z(  ( Sin2Y * Sin2P * Cos2R ) - ( Cos2Y * Cos2P * Sin2R ) );
}

BcVec3d BcQuat::asEular() const
{
	//
	BcReal Sp = -2.0f * ( y() * z() - w() * x() );

	if( BcAbs( Sp ) > 0.9999f )
	{
		BcReal Pitch = ( BcPI * 0.5f ) * Sp;
		BcReal Yaw = BcAtan2( -x() * z() + w() * y(), 0.5f - y() * y() - z() * z() );
		BcReal Roll = 0.0f;

		return BcVec3d( Pitch, Yaw, Roll );
	}
	else
	{
		BcReal Pitch = BcAsin( Sp );
		BcReal Yaw = BcAtan2( x() * z() + w() * y(), 0.5f - x() * x() - y() * y() );
		BcReal Roll = BcAtan2( x() * y() + w() * z(), 0.5f - x() * x() - z() * z() );

		return BcVec3d( Pitch, Yaw, Roll );
	}
}
