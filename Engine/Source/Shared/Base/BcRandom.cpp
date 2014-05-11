/**************************************************************************
*
* File:		BcRandom.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*
*
*
*
* 
**************************************************************************/

#include "Base/BcRandom.h"

#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// Global rng.
BcRandom BcRandom::Global;

//////////////////////////////////////////////////////////////////////////
// Ctor
BcRandom::BcRandom()
{
	Seed_ = 123;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcRandom::BcRandom( BcU32 Seed )
{
	Seed_ = Seed;
}

//////////////////////////////////////////////////////////////////////////
// rand
BcS32 BcRandom::rand()
{
	BcS32 Hi = Seed_ / 127773;
	BcS32 Lo = Seed_ % 127773;

	if( ( Seed_ = 16807 * Lo - 2836 * Hi ) <= 0 )
	{
		Seed_ += 2147483647;
	}
	return Seed_;
}

//////////////////////////////////////////////////////////////////////////
// randReal
BcF32 BcRandom::randReal()
{
	return BcF32( randRange( (BcS32)-4096.0f, (BcS32)4096.0f ) ) / 4096.0f;
}

//////////////////////////////////////////////////////////////////////////
// randRange
BcS32 BcRandom::randRange( BcS32 Min, BcS32 Max )
{
	return BcClamp( ( Min + ( rand() % ( 1 + Max - Min ) ) ), Min, Max );
}

//////////////////////////////////////////////////////////////////////////
// noise
BcF32 BcRandom::noise( BcU32 X, BcU32 Width )
{
	X = X % Width;
	X = ( X << 13 ) ^ X;
	return ( 1.0f - ( ( X * ( X * X * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f );
}

//////////////////////////////////////////////////////////////////////////
// smoothedNoise
BcF32 BcRandom::smoothedNoise( BcF32 X, BcU32 Width )
{
	return noise( (BcU32)X, Width ) / 2.0f + noise( (BcU32)X - 1, Width ) / 4 + noise( (BcU32)X + 1, Width ) / 4;
}

//////////////////////////////////////////////////////////////////////////
// interpolatedNoise
BcF32 BcRandom::interpolatedNoise( BcF32 X, BcU32 Width )
{
	BcU32 iX = BcU32( X );
	BcF32 FracX = X - iX;
	BcF32 V1 = smoothedNoise( (BcF32)iX, Width );
	BcF32 V2 = smoothedNoise( (BcF32)iX + 1.0f, Width );
	return V1 + ( V2 - V1 ) * FracX;
}
