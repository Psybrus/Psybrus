/**************************************************************************
*
* File:		SsEnvironment.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Sound environment.
*		
*
*
* 
**************************************************************************/

#include "SsEnvironment.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
SsEnvironment::SsEnvironment()
{

}

////////////////////////////////////////////////////////////////////////////////
// Dtor
SsEnvironment::~SsEnvironment()
{

}

////////////////////////////////////////////////////////////////////////////////
// interpolate
void SsEnvironment::interpolate( SsEnvironment* pEnvA, SsEnvironment* pEnvB, BcReal Amount )
{
	Density_ = BcLerp( pEnvA->Density_, pEnvB->Density_, Amount );
	Diffusion_ = BcLerp( pEnvA->Diffusion_, pEnvB->Diffusion_, Amount );
	Gain_ = BcLerp( pEnvA->Gain_, pEnvB->Gain_, Amount );
	GainHF_ = BcLerp( pEnvA->GainHF_, pEnvB->GainHF_, Amount );
	DecayTime_ = BcLerp( pEnvA->DecayTime_, pEnvB->DecayTime_, Amount );
	DecayHFRatio_ = BcLerp( pEnvA->DecayHFRatio_, pEnvB->DecayHFRatio_, Amount );
	ReflectionsGain_ = BcLerp( pEnvA->ReflectionsGain_, pEnvB->ReflectionsGain_, Amount );
	ReflectionsDelay_ = BcLerp( pEnvA->ReflectionsDelay_, pEnvB->ReflectionsDelay_, Amount );
	LateReverbGain_ = BcLerp( pEnvA->LateReverbGain_, pEnvB->LateReverbGain_, Amount );
	LateReverbDelay_ = BcLerp( pEnvA->LateReverbDelay_, pEnvB->LateReverbDelay_, Amount );
	AirAbsorptionGainHF_ = BcLerp( pEnvA->AirAbsorptionGainHF_, pEnvB->AirAbsorptionGainHF_, Amount );
	RoomRolloffFactor_ = BcLerp( pEnvA->RoomRolloffFactor_, pEnvB->RoomRolloffFactor_, Amount );
}

////////////////////////////////////////////////////////////////////////////////
// setDefault
void SsEnvironment::setDefault()
{
	Density_ = 1.0f;
	Diffusion_ = 1.0f;
	Gain_ = 0.32f;
	GainHF_ = 0.89f;
	DecayTime_ = 1.49f;
	DecayHFRatio_ = 0.83f;
	ReflectionsGain_ = 0.05f;
	ReflectionsDelay_ = 0.007f;
	LateReverbGain_ = 1.26f;
	LateReverbDelay_ = 0.011f;
	AirAbsorptionGainHF_ = 0.994f;
	RoomRolloffFactor_ = 0.0f;
}

////////////////////////////////////////////////////////////////////////////////
// clamp
void SsEnvironment::clamp()
{
	Density_ = BcClamp( Density_, 0.0f, 1.0f );
	Diffusion_ = BcClamp( Diffusion_, 0.0f, 1.0f );
	Gain_ = BcClamp( Gain_, 0.0f, 1.0f );
	GainHF_ = BcClamp( GainHF_, 0.0f, 1.0f );
	DecayTime_ = BcClamp( DecayTime_, 0.1f, 20.0f );
	DecayHFRatio_ = BcClamp( DecayHFRatio_, 0.1f, 2.0f );
	ReflectionsGain_ = BcClamp( ReflectionsGain_, 0.0f, 3.16f );
	ReflectionsDelay_ = BcClamp( ReflectionsDelay_, 0.0f, 0.3f );
	LateReverbGain_ = BcClamp( LateReverbGain_, 0.0f, 10.0f );
	LateReverbDelay_ = BcClamp( LateReverbDelay_, 0.0f, 0.1f );
	AirAbsorptionGainHF_ = BcClamp( AirAbsorptionGainHF_, 0.892f, 1.0f );
	RoomRolloffFactor_ = BcClamp( RoomRolloffFactor_, 0.0f, 10.0f );
}
