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

#include "BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// BcRandom
class BcRandom
{
public:
	BcRandom();
	BcRandom( BcU32 Seed );

	BcU32 rand();
	BcReal randReal();

	BcU32 randRange( BcU32 Min, BcU32 Max );

private:
	BcU32 Z_;
	BcU32 W_;
};



#endif
