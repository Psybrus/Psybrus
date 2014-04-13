/**************************************************************************
*
* File:		BcCPUVecQuad.h
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*
*
*
*
*
**************************************************************************/

#ifndef __BCCPUVECQUADH__
#define __BCCPUVECQUADH__

#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// BcCPUVecQuad
struct BcCPUVecQuad
{
public:
	BcForceInline BcCPUVecQuad()
	{}

	BcForceInline BcCPUVecQuad( BcF32 X, BcF32 Y ):
		X_( X ),
		Y_( Y )
	{}

	BcForceInline BcCPUVecQuad( BcF32 X, BcF32 Y, BcF32 Z ):
		X_( X ),
		Y_( Y ),
		Z_( Z )
	{}

	BcForceInline BcCPUVecQuad( BcF32 X, BcF32 Y, BcF32 Z, BcF32 W ):
		X_( X ),
		Y_( Y ),
		Z_( Z ),
		W_( W )
	{}

protected:
	BcAlign( BcF32 X_, 16 );
	BcF32 Y_;
	BcF32 Z_;
	BcF32 W_;
};


#endif // __BCCPUVECQUADH__
