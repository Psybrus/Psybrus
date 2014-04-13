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
