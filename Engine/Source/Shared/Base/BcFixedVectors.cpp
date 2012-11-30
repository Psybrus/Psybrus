/**************************************************************************
*
* File:		Vectors.cpp
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		
*
*
*
* 
**************************************************************************/

#include "Base/BcFixedVectors.h"

#include "Base/BcString.h"

void BcFixedVec2d::lerp(const BcFixedVec2d& A, const BcFixedVec2d& B, BcFixed<> T)
{
	const BcFixed<> lK0 = 1.0f - T;
	const BcFixed<> lK1 = T;

	set((A.X_ * lK0) + (B.X_ * lK1),
	    (A.Y_ * lK0) + (B.Y_ * lK1));
}
	
void BcFixedVec3d::lerp(const BcFixedVec3d& A, const BcFixedVec3d& B, BcFixed<> T)
{
	const BcFixed<> lK0 = 1.0f - T;
	const BcFixed<> lK1 = T;

	set((A.X_ * lK0) + (B.X_ * lK1),
		(A.Y_ * lK0) + (B.Y_ * lK1),
		(A.Z_ * lK0) + (B.Z_ * lK1));
}

void BcFixedVec4d::lerp(const BcFixedVec4d& A, const BcFixedVec4d& B, BcFixed<> T)
{
	const BcFixed<> lK0 = 1.0f - T;
	const BcFixed<> lK1 = T;

	set((A.X_ * lK0) + (B.X_ * lK1),
		(A.Y_ * lK0) + (B.Y_ * lK1),
		(A.Z_ * lK0) + (B.Z_ * lK1),
		(A.W_ * lK0) + (B.W_ * lK1));
}
