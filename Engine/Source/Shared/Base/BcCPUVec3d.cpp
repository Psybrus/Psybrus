/**************************************************************************
*
* File:		BcCPUVec3d.cpp
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*
*
*
*
*
**************************************************************************/

#include "Base/BcCPUVec3d.h"
#include "Base/BcCPUVec2d.h"
#include "Base/BcMath.h"

#include "Base/BcString.h"

BcCPUVec3d::BcCPUVec3d( const BcChar* pString )
{
	BcSScanf( pString, "%f,%f,%f", &X_, &Y_, &Z_ );
}

BcCPUVec3d::BcCPUVec3d( const BcCPUVec2d& V, BcF32 Z ):
	BcCPUVecQuad( V.x(), V.y(), Z )
{

}


void BcCPUVec3d::lerp(const BcCPUVec3d& A, const BcCPUVec3d& B, BcF32 T)
{
	const BcF32 lK0 = 1.0f - T;
	const BcF32 lK1 = T;

	set((A.X_ * lK0) + (B.X_ * lK1),
		(A.Y_ * lK0) + (B.Y_ * lK1),
		(A.Z_ * lK0) + (B.Z_ * lK1));
}

BcF32 BcCPUVec3d::magnitude() const
{
	return BcSqrt( magnitudeSquared() );
}

BcF32 BcCPUVec3d::magnitudeSquared() const
{
	return ( X_ * X_ ) + ( Y_ * Y_ ) + ( Z_ * Z_ );
}

BcCPUVec3d BcCPUVec3d::normal() const
{
	BcF32 Mag = magnitude();

	if ( Mag == 0.0f )
	{
		return BcCPUVec3d(0,0,0);
	}

	const BcF32 InvMag = 1.0f / Mag;
	return BcCPUVec3d(X_ * InvMag, Y_ * InvMag, Z_ * InvMag);
}

void BcCPUVec3d::normalise()
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

BcCPUVec3d BcCPUVec3d::cross( const BcCPUVec3d& Rhs ) const
{
	return BcCPUVec3d( ( Y_ * Rhs.Z_)  - ( Rhs.Y_ * Z_ ), ( Z_* Rhs.X_ ) - ( Rhs.Z_ * X_ ), ( X_ * Rhs.Y_ ) - ( Rhs.X_ * Y_ ) );
}

BcBool BcCPUVec3d::operator == (const BcCPUVec3d &Rhs) const
{
	return ( ( BcAbs( X_ - Rhs.X_ ) < BcVecEpsilon ) &&
	         ( BcAbs( Y_ - Rhs.Y_ ) < BcVecEpsilon ) &&
	         ( BcAbs( Z_ - Rhs.Z_ ) < BcVecEpsilon ) );
}

BcCPUVec3d BcCPUVec3d::reflect( const BcCPUVec3d& Normal ) const
{
	return ( *this - ( Normal * ( 2.0f * this->dot( Normal ) ) ) );
}

BcBool BcCheckFloat( BcCPUVec3d T )
{
	return BcCheckFloat( T.x() ) && BcCheckFloat( T.y() ) && BcCheckFloat( T.z() );
}
