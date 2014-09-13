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
	BcF32 randRealRange( BcF32 Min, BcF32 Max );
	BcS32 randRange( BcS32 Min, BcS32 Max );
	BcF32 noise( BcU32 X, BcU32 Width );
	BcF32 smoothedNoise( BcF32 X, BcU32 Width = 512 );
	BcF32 interpolatedNoise( BcF32 X, BcU32 Width = 512 );

private:
	BcU32 Seed_;
};



#endif
