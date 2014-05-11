/**************************************************************************
*
* File:		MaQuat.cpp
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		
*		
*		
*		
* 
**************************************************************************/

#include "Base/BcDebug.h"
#include "Math/MaQuat.h"
#include "Base/BcMath.h"

#include "Base/BcString.h"

#include <boost/format.hpp>

void MaQuat::StaticRegisterClass()
{
	class MaQuatSerialiser:
		public ReClassSerialiser_ComplexType< MaQuat >
	{
	public:
		MaQuatSerialiser( BcName Name ): 
			ReClassSerialiser_ComplexType< MaQuat >( Name )
		{}

		virtual BcBool serialiseToString( const void* pInstance, std::string& OutString ) const
		{
			const MaQuat& Vec = *reinterpret_cast< const MaQuat* >( pInstance );
			OutString = boost::str( boost::format( "%1%, %2%, %3%, %4%" ) % Vec.x() % Vec.y() % Vec.z() % Vec.w() );
			return true;
		}

		virtual BcBool serialiseFromString( void* pInstance, const std::string& InString ) const
		{
			MaQuat& Vec = *reinterpret_cast< MaQuat* >( pInstance );
			Vec = MaQuat( InString.c_str() );
			return true;
		}

		virtual BcBool serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const
		{
			const MaQuat& Vec = *reinterpret_cast< const MaQuat* >( pInstance );
			Serialiser << Vec;
			return true;
		}

		virtual BcBool serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const 
		{
			MaQuat& Vec = *reinterpret_cast< MaQuat* >( pInstance );
			Serialiser >> Vec;
			return true;
		}

		virtual BcBool copy( void* pDst, void* pSrc ) const
		{
			MaQuat& Dst = *reinterpret_cast< MaQuat* >( pDst );
			MaQuat& Src = *reinterpret_cast< MaQuat* >( pSrc );
			Dst = Src;
			return true;
		}
	};

	ReRegisterClass< MaQuat >( new MaQuatSerialiser( "class MaQuat" ) );
}

MaQuat::MaQuat( const BcChar* pString ):
	MaVec4d( pString )
{

}

#define SLERP_EPSILON	0.001f

void MaQuat::lerp(const MaQuat& a, const MaQuat& b, BcF32 t)
{
	BcF32 lK0 = 1.0f - t;
	BcF32 lK1 = t;
	
	set((a.x() * lK0) + (b.x() * lK1),
	    (a.y() * lK0) + (b.y() * lK1),
	    (a.z() * lK0) + (b.z() * lK1),
	    (a.w() * lK0) + (b.w() * lK1));
}
	
void MaQuat::slerp( const MaQuat& a, const MaQuat& b, BcF32 t)
{
	// The following code is based on what Dunn & Parberry do.
	BcF32 lCosOmega = (a.w() * b.w()) + (a.x() * b.x()) + (a.y() * b.y()) + (a.z() * b.z());
	BcF32 lK0;
	BcF32 lK1;
	MaQuat a2 = a;

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
		BcF32 lSinOmega = BcSqrt(1.0f - (lCosOmega * lCosOmega));
		BcF32 lOmega = atan2f(lSinOmega, lCosOmega);
		BcF32 lInvSinOmega = (1.0f / lSinOmega);

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
void MaQuat::fromMatrix4d( const MaMat4d& Mat )
{
	BcF32 FourWSqMinus1 = Mat[0][0] + Mat[1][1] + Mat[2][2];
	BcF32 FourXSqMinus1 = Mat[0][0] - Mat[1][1] - Mat[2][2];
	BcF32 FourYSqMinus1 = Mat[1][1] - Mat[0][0] - Mat[2][2];
	BcF32 FourZSqMinus1 = Mat[2][2] - Mat[0][0] - Mat[1][1];

	BcU32 BiggestIndex = 0;
	BcF32 FourBiggestSqMinus1 = FourWSqMinus1;
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

	BcF32 BiggestVal = BcSqrt( FourBiggestSqMinus1 + 1.0f ) * 0.5f;
	BcF32 Mult = 0.25f / BiggestVal;

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

void MaQuat::asMatrix4d( MaMat4d& Matrix )  const
{
	// Multiply out the values and store in a variable
	// since storing in variables is quicker than 
	// multiplying floating point numbers again and again.
	// This should make this function a touch faster.
	
	// Set of w() multiplications required
	const BcF32 lWX2 = 2.0f * w() * x();
	const BcF32 lWY2 = 2.0f * w() * y();
	const BcF32 lWZ2 = 2.0f * w() * z();

	// Set of x() multiplications required
	const BcF32 lXX2 = 2.0f * x() * x();
	const BcF32 lXY2 = 2.0f * x() * y();
	const BcF32 lXZ2 = 2.0f * x() * z();

	// Remainder of y() multiplications
	const BcF32 lYY2 = 2.0f * y() * y();
	const BcF32 lYZ2 = 2.0f * y() * z();

	// Remainder of z() multiplications
	const BcF32 lZZ2 = 2.0f * z() * z();
	
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

void MaQuat::calcFromXYZ()
{
	BcF32 t = 1.0f - ( x() * x() )
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

void MaQuat::fromEular( BcF32 Yaw, BcF32 Pitch, BcF32 Roll )
{
	const BcF32 Sin2Y = BcSin( Yaw / 2.0f );
	const BcF32 Cos2Y = BcCos( Yaw / 2.0f );
	const BcF32 Sin2P = BcSin( Pitch / 2.0f );
	const BcF32 Cos2P = BcCos( Pitch / 2.0f );
	const BcF32 Sin2R = BcSin( Roll / 2.0f );
	const BcF32 Cos2R = BcCos( Roll / 2.0f );

	w(  ( Cos2Y * Cos2P * Cos2R ) + ( Sin2Y * Sin2P * Sin2R ) );
	x( -( Cos2Y * Sin2P * Cos2R ) - ( Sin2Y * Cos2P * Sin2R ) );
	y(  ( Cos2Y * Sin2P * Sin2R ) - ( Sin2Y * Cos2P * Cos2R ) );
	z(  ( Sin2Y * Sin2P * Cos2R ) - ( Cos2Y * Cos2P * Sin2R ) );
}

MaVec3d MaQuat::asEular() const
{
	//
	BcF32 Sp = -2.0f * ( y() * z() - w() * x() );

	if( BcAbs( Sp ) > 0.9999f )
	{
		BcF32 Pitch = ( BcPI * 0.5f ) * Sp;
		BcF32 Yaw = BcAtan2( -x() * z() + w() * y(), 0.5f - y() * y() - z() * z() );
		BcF32 Roll = 0.0f;

		return MaVec3d( Pitch, Yaw, Roll );
	}
	else
	{
		BcF32 Pitch = BcAsin( Sp );
		BcF32 Yaw = BcAtan2( x() * z() + w() * y(), 0.5f - x() * x() - y() * y() );
		BcF32 Roll = BcAtan2( x() * y() + w() * z(), 0.5f - x() * x() - z() * z() );

		return MaVec3d( Pitch, Yaw, Roll );
	}
}

void MaQuat::rotateTo( const MaVec3d& From, const MaVec3d& To )
{
	const MaVec3d FromNormalised( From.normal() );
	const MaVec3d ToNormalised( To.normal() );
	const MaVec3d Axis( From.cross( To ) );
	const BcF32 RadSin = BcSqrt( ( 1.0f - ( FromNormalised.dot( To ) ) ) * 0.5f );

	x( RadSin * Axis.x() );
	y( RadSin * Axis.y() );
	z( RadSin * Axis.z() );
	w( BcSqrt( ( 1.0f + ( FromNormalised.dot( To ) ) ) * 0.5f ) );
}

void MaQuat::axisAngle( const MaVec3d& Axis, BcF32 Angle )
{
	const MaVec3d AxisNormalised( Axis.normal() );
	const BcF32 RadSin = BcSin( Angle * 0.5f );

	x( RadSin * AxisNormalised.x() );
	y( RadSin * AxisNormalised.y() );
	z( RadSin * AxisNormalised.z() );
	w( BcCos( Angle * 0.5f ) );	
}

// Arithmetic

// Cross Product
MaQuat MaQuat::operator * (const MaQuat& rhs) const
{
	return MaQuat( (w() * rhs.x()) + (x() * rhs.w()) + (y() * rhs.z()) - (z() * rhs.y()) ,
	               (w() * rhs.y()) + (y() * rhs.w()) + (z() * rhs.x()) - (x() * rhs.z()) ,
	               (w() * rhs.z()) + (z() * rhs.w()) + (x() * rhs.y()) - (y() * rhs.x()) ,
	               (w() * rhs.w()) - (x() * rhs.x()) - (y() * rhs.y()) - (z() * rhs.z()) );
}

// Inverse
MaQuat MaQuat::operator ~ () const
{
	return MaQuat(-x(), -y(), -z(), w());
}

// Additional stuff

// Make an identity quaternion 1.0(0.0, 0.0, 0.0)
void MaQuat::makeIdentity()
{
	x( 0.0f );
	y( 0.0f );
	z( 0.0f );
	w( 1.0f );
}

// Return the magnitude
BcF32 MaQuat::magnitude()
{
	return BcSqrt(w() * w() + x() * x() + y() * y() + z() * z());
}

// Quick speed up - needs optimising
void MaQuat::rotateVector( MaVec3d& vec ) const
{	
	const MaQuat& This = (*this);
	const MaQuat OutVec = This * MaQuat( vec.x(), vec.y(), vec.z(), 1.0f ) * ~This;

	vec.set( OutVec.x(), OutVec.y(), OutVec.z() );       		
}

