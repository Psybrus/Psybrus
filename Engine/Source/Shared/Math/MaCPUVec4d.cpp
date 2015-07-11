/**************************************************************************
*
* File:		MaCPUVec4d.cpp
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		
*
*
*
* 
**************************************************************************/

#include "Math/MaCPUVec4d.h"
#include "Math/MaCPUVec3d.h"
#include "Math/MaCPUVec2d.h"
#include "Math/MaVec4d.h"
#include "Math/MaVec3d.h"
#include "Math/MaVec2d.h"
#include "Base/BcMath.h"

#include "Base/BcString.h"

void MaCPUVec4d::StaticRegisterClass()
{
	class MaCPUVec4dSerialiser:
		public ReClassSerialiser_ComplexType< MaCPUVec4d >
	{
	public:
		MaCPUVec4dSerialiser( BcName Name ): 
			ReClassSerialiser_ComplexType< MaCPUVec4d >( Name )
		{}

		virtual BcBool serialiseToString( const void* pInstance, std::string& OutString ) const
		{
			const MaCPUVec4d& Vec = *reinterpret_cast< const MaCPUVec4d* >( pInstance );
			BcChar OutChars[ 1024 ] = { 0 };
			BcSPrintf( OutChars, sizeof( OutChars ) - 1, "%.16f, %.16f, %.16f, %.16f", Vec.x(), Vec.y(), Vec.z(), Vec.w() );
			OutString = OutChars;
			return true;
		}

		virtual BcBool serialiseFromString( void* pInstance, const std::string& InString ) const
		{
			MaCPUVec4d& Vec = *reinterpret_cast< MaCPUVec4d* >( pInstance );
			Vec = MaCPUVec4d( InString.c_str() );
			return true;
		}

		virtual BcBool serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const
		{
			const MaCPUVec4d& Vec = *reinterpret_cast< const MaCPUVec4d* >( pInstance );
			Serialiser << Vec;
			return true;
		}

		virtual BcBool serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const 
		{
			MaCPUVec4d& Vec = *reinterpret_cast< MaCPUVec4d* >( pInstance );
			Serialiser >> Vec;
			return true;
		}

		virtual BcBool copy( void* pDst, void* pSrc ) const
		{
			MaCPUVec4d& Dst = *reinterpret_cast< MaCPUVec4d* >( pDst );
			MaCPUVec4d& Src = *reinterpret_cast< MaCPUVec4d* >( pSrc );
			Dst = Src;
			return true;
		}
	};

	ReRegisterClass< MaCPUVec4d >( new MaCPUVec4dSerialiser( "class MaVec4d" ) );
}

MaCPUVec4d::MaCPUVec4d( const BcChar* pString )
{
	BcSScanf( pString, "%f,%f,%f,%f", &X_, &Y_, &Z_, &W_ );
}

MaCPUVec4d::MaCPUVec4d( const MaCPUVec2d& Rhs ):
	X_( Rhs.x() ), 
	Y_( Rhs.y() ),
	Z_( 0.0f ),
	W_( 0.0f )
{

}

MaCPUVec4d::MaCPUVec4d( const MaCPUVec3d& Rhs, BcF32 W ):
	X_( Rhs.x() ), 
	Y_( Rhs.y() ),
	Z_( Rhs.z() ),
	W_( W )
{

}

void MaCPUVec4d::lerp(const MaCPUVec4d& A, const MaCPUVec4d& B, BcF32 T)
{
	const BcF32 lK0 = 1.0f - T;
	const BcF32 lK1 = T;

	set((A.X_ * lK0) + (B.X_ * lK1),
		(A.Y_ * lK0) + (B.Y_ * lK1),
		(A.Z_ * lK0) + (B.Z_ * lK1),
		(A.W_ * lK0) + (B.W_ * lK1));
}

MaCPUVec3d MaCPUVec4d::normal3() const
{
	BcF32 Mag = BcSqrt( X_ * X_ + Y_ * Y_ + Z_ * Z_ );
	
	if ( Mag == 0.0f )
	{
		return MaCPUVec3d(0,0,0);
	}
	
	const BcF32 InvMag = 1.0f / Mag;
	return MaCPUVec3d( X_ * InvMag, Y_ * InvMag, Z_ * InvMag );
}

BcF32 MaCPUVec4d::magnitude() const
{
	return BcSqrt( X_ * X_ + Y_ * Y_ + Z_ * Z_ + W_ * W_ );
}

BcF32 MaCPUVec4d::magnitudeSquared() const
{
	return ( X_ * X_ + Y_ * Y_ + Z_ * Z_ + W_ * W_ );
}

void MaCPUVec4d::normalise()
{
	BcF32 Mag = magnitude();
	
	if ( Mag == 0.0f )
	{
		return;
	}

	const BcF32 InvMag = 1.0f / Mag;
	X_ *= InvMag;
	Y_ *= InvMag;
	Z_ *= InvMag;
	W_ *= InvMag;
}

void MaCPUVec4d::normalise3()
{
	BcF32 Mag = BcSqrt( X_ * X_ + Y_ * Y_ + Z_ * Z_ );

	if ( Mag == 0.0f )
	{
		return;
	}

	const BcF32 InvMag = 1.0f / Mag;
	X_ *= InvMag;
	Y_ *= InvMag;
	Z_ *= InvMag;
}

MaCPUVec4d MaCPUVec4d::normal() const
{
	BcF32 Mag = magnitude();
	
	if ( Mag == 0.0f )
	{
		return MaCPUVec4d(0,0,0,0);
	}
	
	const BcF32 InvMag = 1.0f / Mag;
	return MaCPUVec4d( X_ * InvMag, Y_ * InvMag, Z_ * InvMag, W_ * InvMag );
}

BcBool MaCPUVec4d::operator == ( const MaCPUVec4d &Rhs ) const
{
	return ( ( BcAbs( X_ - Rhs.X_ ) < BcVecEpsilon ) &&
	         ( BcAbs( Y_ - Rhs.Y_ ) < BcVecEpsilon ) &&
			 ( BcAbs( Z_ - Rhs.Z_ ) < BcVecEpsilon ) &&
			 ( BcAbs( W_ - Rhs.W_ ) < BcVecEpsilon ) );
}

BcBool BcCheckFloat( MaCPUVec4d T )
{
	return BcCheckFloat( T.x() ) && BcCheckFloat( T.y() ) && BcCheckFloat( T.z() ) && BcCheckFloat( T.w() );
}
