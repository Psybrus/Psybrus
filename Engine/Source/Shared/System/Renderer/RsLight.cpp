/**************************************************************************
*
* File:		RsLight.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Light object.
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsLight.h"

#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// findAttenuationByDistance
BcF32 RsLight::findAttenuationByDistance( BcF32 Distance ) const
{
	BcF32 InvA = ( ( AttnC_ + ( Distance * AttnL_ ) + ( Distance * Distance * AttnQ_ ) ) );
	return 1.0f / InvA;
}

//////////////////////////////////////////////////////////////////////////
// findDistanceByAttenuation
BcF32 RsLight::findDistanceByAttenuation( BcF32 Attenuation ) const
{
	// If we want it at a low value, return max distance.
	if( Attenuation < 0.05f )
	{
		Attenuation = 0.05f;
	}

	BcF32 A = AttnQ_;
	BcF32 B = AttnL_;
	BcF32 C = AttnC_ - ( 1.0f / Attenuation );

	// 0 = Ax2 + Bx + C
	if( A > 0.0f )
	{
		BcF32 Discriminant = ( ( B * B ) - ( 4.0f * A * C ) );
		if( Discriminant < 0.0f )
		{
			return 1e24f;
		}
		return BcAbs( ( -B + BcSqrt( Discriminant ) ) / ( 2.0f * A ) );
	}
	// 0 = Bx + C
	else if( B > 0.0f )
	{
		return BcAbs( -C / B );
	}
	// 0 = C
	else
	{
		return 1e24f;
	}
}

//////////////////////////////////////////////////////////////////////////
//
void RsLight::createAttenuationValues( BcF32 MinDistance, BcF32 MidDistance, BcF32 MaxDistance )
{
	// x = min - 1 ( 1 )
	// y = mid - 12 ( 144 )
	// z = max - 16 ( 256 )
	//
	// Ax^2 + Bx + C = 1.0f / 1.0f
	// Ay^2 + By + C = 1.0f / 0.5f
	// Az^2 + Bz + C = 1.0f / 0.1f
	//
	// Solve for A, B and C. x, y & z are all known positive values.

	// TODO: Implement MaxDistance. Only linear is produced from this.
	AttnQ_ = 0.0f;
	AttnL_ = 1.0f / ( MidDistance - MinDistance );
	AttnC_ = ( 1.0f - MinDistance * AttnL_ );

	// Post-conditions:
	// findAttenuationByDistance( MinDistance ) == 1.0f +/- Epsilon
	// findAttenuationByDistance( MidDistance ) == 0.5f +/- Epsilon
	// findAttenuationByDistance( MaxDistance ) == 0.01f +/- Epsilon
}
