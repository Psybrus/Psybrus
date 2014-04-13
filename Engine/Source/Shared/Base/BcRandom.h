/**************************************************************************
*
* File:		BcRandom.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Marsaglia's MWC.
*		TODO: Perhaps implement something a bit, well, better?
*
*
* 
**************************************************************************/

#ifndef __BCRANDOM_H__
#define __BCRANDOM_H__

#include "Base/BcTypes.h"
#include "Base/BcVec2d.h"
#include "Base/BcVec3d.h"
#include "Base/BcVec4d.h"

//////////////////////////////////////////////////////////////////////////
// BcRandom
class BcRandom
{
public:
	static BcRandom Global;

public:
	BcRandom();
	BcRandom( BcU32 Seed );

	BcS32 rand();
	BcF32 randReal();
	BcS32 randRange( BcS32 Min, BcS32 Max );
	BcF32 noise( BcU32 X, BcU32 Width );
	BcF32 smoothedNoise( BcF32 X, BcU32 Width = 512 );
	BcF32 interpolatedNoise( BcF32 X, BcU32 Width = 512 );

	BcVec2d randVec2();
	BcVec3d randVec3();
	BcVec4d randVec4();
	BcVec2d randVec2Normal();
	BcVec3d randVec3Normal();
	BcVec4d randVec4Normal();

private:
	BcU32 Seed_;
};



#endif
