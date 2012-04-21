/**************************************************************************
*
* File:		BcRandom.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Marsaglia's MWC.
*		TODO: Perhaps implement something a bit, well, better?
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
	Z_ = 0x0000dead;
	W_ = 0x0000beef;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcRandom::BcRandom( BcU32 Seed )
{
	Z_ = Seed & 65535;
	W_ = Seed >> 16;
}

//////////////////////////////////////////////////////////////////////////
// rand
BcU32 BcRandom::rand()
{
	Z_ = 36969 * ( Z_ & 65535 ) + ( Z_ >> 16 );
	W_ = 18000 * ( W_ & 65535 ) + ( W_ >> 16 );

	return ( Z_ << 16 ) + W_;
}

//////////////////////////////////////////////////////////////////////////
// randReal
BcReal BcRandom::randReal()
{
	return BcReal( rand() ) * 2.328306435996595e-10f;
}

//////////////////////////////////////////////////////////////////////////
// randRange
BcU32 BcRandom::randRange( BcU32 Min, BcU32 Max )
{
	return BcClamp( ( Min + ( rand() % ( Max - Min ) ) ), Min, Max );
}

//////////////////////////////////////////////////////////////////////////
// noise
BcReal BcRandom::noise( BcU32 X, BcU32 Width )
{
	X = X % Width;
	X = ( X << 13 ) ^ X;
	return ( 1.0f - ( ( X * ( X * X * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f );
}

//////////////////////////////////////////////////////////////////////////
// smoothedNoise
BcReal BcRandom::smoothedNoise( BcReal X, BcU32 Width )
{
	return noise( (BcU32)X, Width ) / 2.0f + noise( (BcU32)X - 1, Width ) / 4 + noise( (BcU32)X + 1, Width ) / 4;
}

//////////////////////////////////////////////////////////////////////////
// interpolatedNoise
BcReal BcRandom::interpolatedNoise( BcReal X, BcU32 Width )
{
	BcU32 iX = BcU32( X );
	BcReal FracX = X - iX;
	BcReal V1 = smoothedNoise( (BcReal)iX, Width );
	BcReal V2 = smoothedNoise( (BcReal)iX + 1.0f, Width );
	return V1 + ( V2 - V1 ) * FracX;
}

//////////////////////////////////////////////////////////////////////////
// interpolatedNoise
BcVec2d BcRandom::randVec2Normal()
{
	return BcVec2d( randReal() - 0.5f, randReal() - 0.5f ).normal();
}

//////////////////////////////////////////////////////////////////////////
// interpolatedNoise
BcVec3d BcRandom::randVec3Normal()
{
	return BcVec3d( randReal() - 0.5f, randReal() - 0.5f, randReal() - 0.5f ).normal();
}

//////////////////////////////////////////////////////////////////////////
// interpolatedNoise
BcVec4d BcRandom::randVec4Normal()
{
	return BcVec4d( randReal() - 0.5f, randReal() - 0.5f, randReal() - 0.5f, randReal() - 0.5f ).normal();
}
