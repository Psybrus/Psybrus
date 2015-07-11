/**************************************************************************
*
* File:		MaCPUVec3d.cpp
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*
*
*
*
*
**************************************************************************/

#include "Math/MaCPUVec3d.h"
#include "Math/MaCPUVec2d.h"
#include "Math/MaVec3d.h"
#include "Math/MaVec2d.h"
#include "Base/BcMath.h"

#include "Base/BcString.h"


void MaCPUVec3d::StaticRegisterClass()
{
	class MaCPUVec3dSerialiser:
		public ReClassSerialiser_ComplexType< MaCPUVec3d >
	{
	public:
		MaCPUVec3dSerialiser( BcName Name ): 
			ReClassSerialiser_ComplexType< MaCPUVec3d >( Name )
		{}

		virtual BcBool serialiseToString( const void* pInstance, std::string& OutString ) const
		{
			const MaCPUVec3d& Vec = *reinterpret_cast< const MaCPUVec3d* >( pInstance );
			BcChar OutChars[ 1024 ] = { 0 };
			snprintf( OutChars, 1023, "%.16f, %.16f, %.16f", Vec.x(), Vec.y(), Vec.z() );
			OutString = OutChars;
			return true;
		}

		virtual BcBool serialiseFromString( void* pInstance, const std::string& InString ) const
		{
			MaCPUVec3d& Vec = *reinterpret_cast< MaCPUVec3d* >( pInstance );
			Vec = MaCPUVec3d( InString.c_str() );
			return true;
		}

		virtual BcBool serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const
		{
			const MaCPUVec3d& Vec = *reinterpret_cast< const MaCPUVec3d* >( pInstance );
			Serialiser << Vec;
			return true;
		}

		virtual BcBool serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const 
		{
			MaCPUVec3d& Vec = *reinterpret_cast< MaCPUVec3d* >( pInstance );
			Serialiser >> Vec;
			return true;
		}

		virtual BcBool copy( void* pDst, void* pSrc ) const
		{
			MaCPUVec3d& Dst = *reinterpret_cast< MaCPUVec3d* >( pDst );
			MaCPUVec3d& Src = *reinterpret_cast< MaCPUVec3d* >( pSrc );
			Dst = Src;
			return true;
		}
	};

	ReRegisterClass< MaCPUVec3d >( new MaCPUVec3dSerialiser( "class MaVec3d" ) );
}

MaCPUVec3d::MaCPUVec3d( const BcChar* pString )
{
	BcSScanf( pString, "%f,%f,%f", &X_, &Y_, &Z_ );
}

MaCPUVec3d::MaCPUVec3d( const MaCPUVec2d& V, BcF32 Z ):
	X_( V.x() ),
	Y_( V.y() ),
	Z_( Z )
{

}


void MaCPUVec3d::lerp(const MaCPUVec3d& A, const MaCPUVec3d& B, BcF32 T)
{
	const BcF32 lK0 = 1.0f - T;
	const BcF32 lK1 = T;

	set((A.X_ * lK0) + (B.X_ * lK1),
		(A.Y_ * lK0) + (B.Y_ * lK1),
		(A.Z_ * lK0) + (B.Z_ * lK1));
}

BcF32 MaCPUVec3d::magnitude() const
{
	return BcSqrt( magnitudeSquared() );
}

BcF32 MaCPUVec3d::magnitudeSquared() const
{
	return ( X_ * X_ ) + ( Y_ * Y_ ) + ( Z_ * Z_ );
}

MaCPUVec3d MaCPUVec3d::normal() const
{
	BcF32 Mag = magnitude();

	if ( Mag == 0.0f )
	{
		return MaCPUVec3d(0,0,0);
	}

	const BcF32 InvMag = 1.0f / Mag;
	return MaCPUVec3d(X_ * InvMag, Y_ * InvMag, Z_ * InvMag);
}

void MaCPUVec3d::normalise()
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
}

MaCPUVec3d MaCPUVec3d::cross( const MaCPUVec3d& Rhs ) const
{
	return MaCPUVec3d( ( Y_ * Rhs.Z_)  - ( Rhs.Y_ * Z_ ), ( Z_* Rhs.X_ ) - ( Rhs.Z_ * X_ ), ( X_ * Rhs.Y_ ) - ( Rhs.X_ * Y_ ) );
}

BcBool MaCPUVec3d::operator == (const MaCPUVec3d &Rhs) const
{
	return ( ( BcAbs( X_ - Rhs.X_ ) < BcVecEpsilon ) &&
	         ( BcAbs( Y_ - Rhs.Y_ ) < BcVecEpsilon ) &&
	         ( BcAbs( Z_ - Rhs.Z_ ) < BcVecEpsilon ) );
}

MaCPUVec3d MaCPUVec3d::reflect( const MaCPUVec3d& Normal ) const
{
	return ( *this - ( Normal * ( 2.0f * this->dot( Normal ) ) ) );
}

BcBool BcCheckFloat( MaCPUVec3d T )
{
	return BcCheckFloat( T.x() ) && BcCheckFloat( T.y() ) && BcCheckFloat( T.z() );
}
