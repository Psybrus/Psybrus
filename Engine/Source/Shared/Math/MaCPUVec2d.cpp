/**************************************************************************
*
* File:		MaCPUVec2d.cpp
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*
*
*
*
*
**************************************************************************/

#include "Math/MaCPUVec2d.h"
#include "Math/MaVec2d.h"
#include "Base/BcMath.h"

#include "Base/BcString.h"

#include <boost/format.hpp>

void MaCPUVec2d::StaticRegisterClass()
{
	class MaCPUVec2dSerialiser:
		public ReClassSerialiser_ComplexType< MaCPUVec2d >
	{
	public:
		MaCPUVec2dSerialiser( BcName Name ): 
			ReClassSerialiser_ComplexType< MaCPUVec2d >( Name )
		{}

		virtual BcBool serialiseToString( const void* pInstance, std::string& OutString ) const
		{
			const MaCPUVec2d& Vec = *reinterpret_cast< const MaCPUVec2d* >( pInstance );
			OutString = boost::str( boost::format( "%1%, %2%" ) % Vec.x() % Vec.y() );
			return true;
		}

		virtual BcBool serialiseFromString( void* pInstance, const std::string& InString ) const
		{
			MaCPUVec2d& Vec = *reinterpret_cast< MaCPUVec2d* >( pInstance );
			Vec = MaCPUVec2d( InString.c_str() );
			return true;
		}

		virtual BcBool serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const
		{
			const MaCPUVec2d& Vec = *reinterpret_cast< const MaCPUVec2d* >( pInstance );
			Serialiser << Vec;
			return true;
		}

		virtual BcBool serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const 
		{
			MaCPUVec2d& Vec = *reinterpret_cast< MaCPUVec2d* >( pInstance );
			Serialiser >> Vec;
			return true;
		}

		virtual BcBool copy( void* pDst, void* pSrc ) const
		{
			MaCPUVec2d& Dst = *reinterpret_cast< MaCPUVec2d* >( pDst );
			MaCPUVec2d& Src = *reinterpret_cast< MaCPUVec2d* >( pSrc );
			Dst = Src;
			return true;
		}
	};

	ReRegisterClass< MaCPUVec2d >( new MaCPUVec2dSerialiser( "class MaVec2d" ) );
}

MaCPUVec2d::MaCPUVec2d( const BcChar* pString )
{
	BcSScanf( pString, "%f,%f", &X_, &Y_ );
}

void MaCPUVec2d::lerp(const MaCPUVec2d& A, const MaCPUVec2d& B, BcF32 T)
{
	const BcF32 lK0 = 1.0f - T;
	const BcF32 lK1 = T;

	set((A.X_ * lK0) + (B.X_ * lK1),
	    (A.Y_ * lK0) + (B.Y_ * lK1));
}

BcF32 MaCPUVec2d::magnitudeSquared() const
{
	return ( ( X_ * X_ ) + ( Y_ * Y_ ) );
}

BcF32 MaCPUVec2d::magnitude() const
{
	return BcSqrt( magnitudeSquared() );
}

MaCPUVec2d MaCPUVec2d::normal() const
{
	BcF32 Mag = magnitude();
	
	if ( Mag == 0.0f )
	{
		return MaCPUVec2d(0,0);
	}
	
	const BcF32 InvMag = 1.0f / Mag;
	return MaCPUVec2d( X_ * InvMag, Y_ * InvMag );
}

void MaCPUVec2d::normalise()
{
	BcF32 Mag = magnitude();
	
	if ( Mag == 0.0f )
	{
		return;
	}
	
	const BcF32 InvMag = 1.0f / Mag;
	X_ *= InvMag;
	Y_ *= InvMag;
}

BcBool MaCPUVec2d::operator == ( const MaCPUVec2d& Rhs ) const
{
	return ( ( BcAbs( X_ - Rhs.X_ ) < BcVecEpsilon ) &&
	         ( BcAbs( Y_ - Rhs.Y_ ) < BcVecEpsilon ) );
}

BcBool BcCheckFloat( MaCPUVec2d T )
{
	return BcCheckFloat( T.x() ) && BcCheckFloat( T.y() );
}
