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

#include "BcVectors.h"

void BcVec2d::lerp(const BcVec2d& A, const BcVec2d& B, BcReal T)
{
	const BcReal lK0 = 1.0f - T;
	const BcReal lK1 = T;

	set((A.X_ * lK0) + (B.X_ * lK1),
	    (A.Y_ * lK0) + (B.Y_ * lK1));
}
	

void BcVec3d::lerp(const BcVec3d& A, const BcVec3d& B, BcReal T)
{
	const BcReal lK0 = 1.0f - T;
	const BcReal lK1 = T;

	set((A.X_ * lK0) + (B.X_ * lK1),
		(A.Y_ * lK0) + (B.Y_ * lK1),
		(A.Z_ * lK0) + (B.Z_ * lK1));
}

void BcVec4d::lerp(const BcVec4d& A, const BcVec4d& B, BcReal T)
{
	const BcReal lK0 = 1.0f - T;
	const BcReal lK1 = T;

	set((A.X_ * lK0) + (B.X_ * lK1),
		(A.Y_ * lK0) + (B.Y_ * lK1),
		(A.Z_ * lK0) + (B.Z_ * lK1),
		(A.W_ * lK0) + (B.W_ * lK1));
}
