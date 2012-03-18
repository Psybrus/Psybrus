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

	BcU32 rand();
	BcReal randReal();
	BcU32 randRange( BcU32 Min, BcU32 Max );
	BcReal noise( BcU32 X, BcU32 Width );
	BcReal smoothedNoise( BcReal X, BcU32 Width = 512 );
	BcReal interpolatedNoise( BcReal X, BcU32 Width = 512 );

private:
	BcU32 Z_;
	BcU32 W_;
};



#endif
