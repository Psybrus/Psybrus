/**************************************************************************
*
* File:		BcCPUVec2d.cpp
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*
*
*
*
*
**************************************************************************/

#include "Base/BcCPUVec2d.h"
#include "Base/BcMath.h"

#include "Base/BcString.h"

BcCPUVec2d::BcCPUVec2d( const BcChar* pString )
{
	BcSScanf( pString, "%f,%f", &X_, &Y_ );
}

void BcCPUVec2d::lerp(const BcCPUVec2d& A, const BcCPUVec2d& B, BcF32 T)
{
	const BcF32 lK0 = 1.0f - T;
	const BcF32 lK1 = T;

	set((A.X_ * lK0) + (B.X_ * lK1),
	    (A.Y_ * lK0) + (B.Y_ * lK1));
}

BcF32 BcCPUVec2d::magnitudeSquared() const
{
	return ( ( X_ * X_ ) + ( Y_ * Y_ ) );
}

BcF32 BcCPUVec2d::magnitude() const
{
	return BcSqrt( magnitudeSquared() );
}

BcCPUVec2d BcCPUVec2d::normal() const
{
	BcF32 Mag = magnitude();
	
	if ( Mag == 0.0f )
	{
		return BcCPUVec2d(0,0);
	}
	
	const BcF32 InvMag = 1.0f / Mag;
	return BcCPUVec2d( X_ * InvMag, Y_ * InvMag );
}

void BcCPUVec2d::normalise()
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

BcBool BcCPUVec2d::operator == ( const BcCPUVec2d& Rhs ) const
{
	return ( ( BcAbs( X_ - Rhs.X_ ) < BcVecEpsilon ) &&
	         ( BcAbs( Y_ - Rhs.Y_ ) < BcVecEpsilon ) );
}

BcBool BcCheckFloat( BcCPUVec2d T )
{
	return BcCheckFloat( T.x() ) && BcCheckFloat( T.y() );
}
