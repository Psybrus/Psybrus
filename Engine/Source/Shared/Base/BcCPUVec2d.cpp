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
	
