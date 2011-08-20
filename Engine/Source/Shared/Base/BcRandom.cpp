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

#include "BcRandom.h"

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
	return ( Min + ( rand() % ( Max - Min ) ) );
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
	return noise( X, Width ) / 2.0f + noise( X - 1, Width ) / 4 + noise( X + 1, Width ) / 4;
}

//////////////////////////////////////////////////////////////////////////
// interpolatedNoise
BcReal BcRandom::interpolatedNoise( BcReal X, BcU32 Width )
{
	BcU32 iX = BcU32( X );
	BcU32 FracX = X - iX;
	BcReal V1 = smoothedNoise( iX, Width );
	BcReal V2 = smoothedNoise( iX + 1, Width );
	return V1 + ( V2 - V1 ) * FracX;
}

