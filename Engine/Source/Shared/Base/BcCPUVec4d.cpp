/**************************************************************************
*
* File:		BcCPUVec4d.cpp
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		
*
*
*
* 
**************************************************************************/

#include "Base/BcCPUVec4d.h"
#include "Base/BcCPUVec3d.h"
#include "Base/BcCPUVec2d.h"
#include "Base/BcMath.h"

#include "Base/BcString.h"

BcCPUVec4d::BcCPUVec4d( const BcChar* pString )
{
	BcSScanf( pString, "%f,%f,%f,%f", &X_, &Y_, &Z_, &W_ );
}

BcCPUVec4d::BcCPUVec4d( const BcCPUVec2d& Rhs ):
	BcCPUVecQuad( Rhs.x(), Rhs.y(), 0.0f, 0.0f )
{

}

BcCPUVec4d::BcCPUVec4d( const BcCPUVec3d& Rhs, BcF32 W ):
	BcCPUVecQuad( Rhs.x(), Rhs.y(), Rhs.z(), W )
{

}

void BcCPUVec4d::lerp(const BcCPUVec4d& A, const BcCPUVec4d& B, BcF32 T)
{
	const BcF32 lK0 = 1.0f - T;
	const BcF32 lK1 = T;

	set((A.X_ * lK0) + (B.X_ * lK1),
		(A.Y_ * lK0) + (B.Y_ * lK1),
		(A.Z_ * lK0) + (B.Z_ * lK1),
		(A.W_ * lK0) + (B.W_ * lK1));
}

BcCPUVec3d BcCPUVec4d::normal3() const
{
	BcF32 Mag = BcSqrt( X_ * X_ + Y_ * Y_ + Z_ * Z_ );
	
	if ( Mag == 0.0f )
	{
		return BcCPUVec3d(0,0,0);
	}
	
	const BcF32 InvMag = 1.0f / Mag;
	return BcCPUVec3d( X_ * InvMag, Y_ * InvMag, Z_ * InvMag );
}

BcF32 BcCPUVec4d::magnitude() const
{
	return BcSqrt( X_ * X_ + Y_ * Y_ + Z_ * Z_ + W_ * W_ );
}

BcF32 BcCPUVec4d::magnitudeSquared() const
{
	return ( X_ * X_ + Y_ * Y_ + Z_ * Z_ + W_ * W_ );
}

void BcCPUVec4d::normalise()
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

void BcCPUVec4d::normalise3()
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

BcCPUVec4d BcCPUVec4d::normal() const
{
	BcF32 Mag = magnitude();
	
	if ( Mag == 0.0f )
	{
		return BcCPUVec4d(0,0,0,0);
	}
	
	const BcF32 InvMag = 1.0f / Mag;
	return BcCPUVec4d( X_ * InvMag, Y_ * InvMag, Z_ * InvMag, W_ * InvMag );
}

BcBool BcCPUVec4d::operator == ( const BcCPUVec4d &Rhs ) const
{
	return ( ( BcAbs( X_ - Rhs.X_ ) < BcVecEpsilon ) &&
	         ( BcAbs( Y_ - Rhs.Y_ ) < BcVecEpsilon ) &&
			 ( BcAbs( Z_ - Rhs.Z_ ) < BcVecEpsilon ) &&
			 ( BcAbs( W_ - Rhs.W_ ) < BcVecEpsilon ) );
}

BcBool BcCheckFloat( BcCPUVec4d T )
{
	return BcCheckFloat( T.x() ) && BcCheckFloat( T.y() ) && BcCheckFloat( T.z() ) && BcCheckFloat( T.w() );
}
