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

#include "System/Sound/SsEnvironment.h"
#include "Base/BcMath.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
SsEnvironment::SsEnvironment()
{
	setDefault();
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
SsEnvironment::~SsEnvironment()
{

}

////////////////////////////////////////////////////////////////////////////////
// interpolate
void SsEnvironment::interpolate( SsEnvironment* pEnvA, SsEnvironment* pEnvB, BcF32 Amount )
{
	Density_ = BcLerp( pEnvA->Density_, pEnvB->Density_, Amount );
	Diffusion_ = BcLerp( pEnvA->Diffusion_, pEnvB->Diffusion_, Amount );
	Gain_ = BcLerp( pEnvA->Gain_, pEnvB->Gain_, Amount );
	GainHF_ = BcLerp( pEnvA->GainHF_, pEnvB->GainHF_, Amount );
	GainLF_ = BcLerp( pEnvA->GainLF_, pEnvB->GainLF_, Amount );
	DecayTime_ = BcLerp( pEnvA->DecayTime_, pEnvB->DecayTime_, Amount );
	DecayHFRatio_ = BcLerp( pEnvA->DecayHFRatio_, pEnvB->DecayHFRatio_, Amount );
	DecayLFRatio_ = BcLerp( pEnvA->DecayLFRatio_, pEnvB->DecayLFRatio_, Amount );
	ReflectionsGain_ = BcLerp( pEnvA->ReflectionsGain_, pEnvB->ReflectionsGain_, Amount );
	ReflectionsDelay_ = BcLerp( pEnvA->ReflectionsDelay_, pEnvB->ReflectionsDelay_, Amount );
	ReflectionsPan_.lerp( pEnvA->ReflectionsPan_, pEnvB->ReflectionsPan_, Amount );
	LateReverbGain_ = BcLerp( pEnvA->LateReverbGain_, pEnvB->LateReverbGain_, Amount );
	LateReverbDelay_ = BcLerp( pEnvA->LateReverbDelay_, pEnvB->LateReverbDelay_, Amount );
	LateReverbPan_.lerp( pEnvA->LateReverbPan_, pEnvB->LateReverbPan_, Amount );
	EchoTime_ = BcLerp( pEnvA->EchoTime_, pEnvB->EchoTime_, Amount );
	EchoDepth_ = BcLerp( pEnvA->EchoDepth_, pEnvB->EchoDepth_, Amount );
	ModulationTime_ = BcLerp( pEnvA->ModulationTime_, pEnvB->ModulationTime_, Amount );
	ModulationDepth_ = BcLerp( pEnvA->ModulationDepth_, pEnvB->ModulationDepth_, Amount );
	AirAbsorptionGainHF_ = BcLerp( pEnvA->AirAbsorptionGainHF_, pEnvB->AirAbsorptionGainHF_, Amount );
	HFReference_ = BcLerp( pEnvA->HFReference_, pEnvB->HFReference_, Amount );
	LFReference_ = BcLerp( pEnvA->LFReference_, pEnvB->LFReference_, Amount );
	RoomRolloffFactor_ = BcLerp( pEnvA->RoomRolloffFactor_, pEnvB->RoomRolloffFactor_, Amount );
	DecayHFLimit_ = BcLerp( pEnvA->DecayHFLimit_, pEnvB->DecayHFLimit_, Amount );

	clamp();
}

////////////////////////////////////////////////////////////////////////////////
// setDefault
void SsEnvironment::setDefault()
{
	Density_ = 1.0f;
	Diffusion_ = 1.0f;
	Gain_ = 0.32f;
	GainHF_ = 0.89f;
	GainLF_ = 1.0f;
	DecayTime_ = 1.49f;
	DecayHFRatio_ = 0.83f;
	DecayLFRatio_ = 1.0f;
	ReflectionsGain_ = 0.05f;
	ReflectionsDelay_ = 0.007f;
	ReflectionsPan_ = MaVec3d( 0.0f, 0.0f, 0.0f );
	LateReverbGain_ = 1.26f;
	LateReverbDelay_ = 0.011f;
	LateReverbPan_ = MaVec3d( 0.0f, 0.0f, 0.0f );
	EchoTime_ = 0.25f;
	EchoDepth_ = 0.0f;
	ModulationTime_ = 0.25f;
	ModulationDepth_ = 0.0f;
	AirAbsorptionGainHF_ = 0.994f;
	HFReference_ = 5000.0f;
	LFReference_ = 250.0f;
	RoomRolloffFactor_ = 0.0f;
	DecayHFLimit_ = 1.0f;

	// HANGER:
	/*
	Density_ = 1.0f;
	Diffusion_ = 1.0f;
	Gain_ = 0.316f;
	GainHF_ = 0.316f;
	GainLF_ = 1.0f;
	DecayTime_ = 10.05f;
	DecayHFRatio_ = 0.23f;
	DecayLFRatio_ = 1.0f;
	ReflectionsGain_ = 0.5f;
	ReflectionsDelay_ = 0.02f;
	ReflectionsPan_ = MaVec3d( 0.0f, 0.0f, 0.0f );
	LateReverbGain_ = 1.256f;
	LateReverbDelay_ = 0.03f;
	LateReverbPan_ = MaVec3d( 0.0f, 0.0f, 0.0f );
	EchoTime_ = 0.25f;
	EchoDepth_ = 0.0f;
	ModulationTime_ = 0.25f;
	ModulationDepth_ = 0.0f;
	AirAbsorptionGainHF_ = 0.994f;
	HFReference_ = 5000.0f;
	LFReference_ = 250.0f;
	RoomRolloffFactor_ = 0.0f;
	DecayHFLimit_ = 1;
	//*/
	
	/*
	// DRUGGED:
	Density_ = 0.42f;
	Diffusion_ = 0.5f;
	Gain_ = 0.316f;
	GainHF_ = 1.0f;
	GainLF_ = 1.0f;
	DecayTime_ = 8.39f;
	DecayHFRatio_ = 1.39f;
	DecayLFRatio_ = 1.0f;
	ReflectionsGain_ = 0.875f;
	ReflectionsDelay_ = 0.002f;
	ReflectionsPan_ = MaVec3d( 0.0f, 0.0f, 0.0f );
	LateReverbGain_ = 3.108f;
	LateReverbDelay_ = 0.03f;
	LateReverbPan_ = MaVec3d( 0.0f, 0.0f, 0.0f );
	EchoTime_ = 0.25f;
	EchoDepth_ = 0.0f;
	ModulationTime_ = 0.25f;
	ModulationDepth_ = 1.0f;
	AirAbsorptionGainHF_ = 0.994f;
	HFReference_ = 5000.0f;
	LFReference_ = 250.0f;
	RoomRolloffFactor_ = 0.0f;
	DecayHFLimit_ = 0;
	//*/
		
	clamp();
}

////////////////////////////////////////////////////////////////////////////////
// clamp
void SsEnvironment::clamp()
{
	Density_ = BcClamp( Density_, 0.0f, 1.0f );
	Diffusion_ = BcClamp( Diffusion_, 0.0f, 1.0f );
	Gain_ = BcClamp( Gain_, 0.0f, 1.0f );
	GainHF_ = BcClamp( GainHF_, 0.0f, 1.0f );
	GainLF_ = BcClamp( GainLF_, 0.0f, 1.0f );
	DecayTime_ = BcClamp( DecayTime_, 0.1f, 20.0f );
	DecayHFRatio_ = BcClamp( DecayHFRatio_, 0.1f, 2.0f );
	DecayLFRatio_ = BcClamp( DecayLFRatio_, 0.1f, 2.0f );
	ReflectionsGain_ = BcClamp( ReflectionsGain_, 0.0f, 3.16f );
	ReflectionsDelay_ = BcClamp( ReflectionsDelay_, 0.0f, 0.3f );
	LateReverbGain_ = BcClamp( LateReverbGain_, 0.0f, 10.0f );
	LateReverbDelay_ = BcClamp( LateReverbDelay_, 0.0f, 0.1f );
	EchoTime_ = BcClamp( EchoTime_, 0.075f, 0.25f );
	EchoDepth_ = BcClamp( EchoDepth_, 0.0f, 1.0f );
	ModulationTime_ = BcClamp( ModulationTime_, 0.04f, 4.0f );
	ModulationDepth_ = BcClamp( ModulationDepth_, 0.0f, 1.0f );
	AirAbsorptionGainHF_ = BcClamp( AirAbsorptionGainHF_, 0.892f, 1.0f );
	HFReference_ = BcClamp( HFReference_, 1000.0f, 20000.0f );
	LFReference_ = BcClamp( LFReference_, 20.0f, 1000.0f );
	RoomRolloffFactor_ = BcClamp( RoomRolloffFactor_, 0.0f, 10.0f );
	DecayHFLimit_ = BcClamp( DecayHFLimit_, 0.0f, 1.0f );

	if( ReflectionsPan_.magnitudeSquared() > 1.0f )
	{
		ReflectionsPan_.normalise();
	}

	if( LateReverbPan_.magnitudeSquared() > 1.0f )
	{
		LateReverbPan_.normalise();
	}
}
