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
BcReal BcRandom::randReal()
{
	return BcReal( randRange( -4096.0f, 4096.0f ) ) / 4096.0f;
}

//////////////////////////////////////////////////////////////////////////
// randRange
BcS32 BcRandom::randRange( BcS32 Min, BcS32 Max )
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
BcVec2d BcRandom::randVec2()
{
	return BcVec2d( randReal(), randReal() );
}

//////////////////////////////////////////////////////////////////////////
// interpolatedNoise
BcVec3d BcRandom::randVec3()
{
	return BcVec3d( randReal(), randReal(), randReal() );
}

//////////////////////////////////////////////////////////////////////////
// interpolatedNoise
BcVec4d BcRandom::randVec4()
{
	return BcVec4d( randReal(), randReal(), randReal(), randReal() );
}


//////////////////////////////////////////////////////////////////////////
// interpolatedNoise
BcVec2d BcRandom::randVec2Normal()
{
	return BcVec2d( randReal(), randReal() ).normal();
}

//////////////////////////////////////////////////////////////////////////
// interpolatedNoise
BcVec3d BcRandom::randVec3Normal()
{
	return BcVec3d( randReal(), randReal(), randReal() ).normal();
}

//////////////////////////////////////////////////////////////////////////
// interpolatedNoise
BcVec4d BcRandom::randVec4Normal()
{
	return BcVec4d( randReal(), randReal(), randReal(), randReal() ).normal();
}
