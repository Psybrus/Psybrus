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
#include "Base/BcMath.h"

#include "Base/BcString.h"

MaCPUVec3d::MaCPUVec3d( const BcChar* pString )
{
	BcSScanf( pString, "%f,%f,%f", &X_, &Y_, &Z_ );
}

MaCPUVec3d::MaCPUVec3d( const MaCPUVec2d& V, BcF32 Z ):
	MaCPUVecQuad( V.x(), V.y(), Z )
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
