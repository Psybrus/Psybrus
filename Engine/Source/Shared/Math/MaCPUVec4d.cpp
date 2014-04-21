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
#include "Base/BcMath.h"

#include "Base/BcString.h"

MaCPUVec4d::MaCPUVec4d( const BcChar* pString )
{
	BcSScanf( pString, "%f,%f,%f,%f", &X_, &Y_, &Z_, &W_ );
}

MaCPUVec4d::MaCPUVec4d( const MaCPUVec2d& Rhs ):
	MaCPUVecQuad( Rhs.x(), Rhs.y(), 0.0f, 0.0f )
{

}

MaCPUVec4d::MaCPUVec4d( const MaCPUVec3d& Rhs, BcF32 W ):
	MaCPUVecQuad( Rhs.x(), Rhs.y(), Rhs.z(), W )
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
