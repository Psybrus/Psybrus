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
#include "Base/BcMath.h"

#include "Base/BcString.h"

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
