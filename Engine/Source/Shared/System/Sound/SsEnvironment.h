/**************************************************************************
*
* File:		SsEnvironment.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Sound environment.
*		
*
*
* 
**************************************************************************/

#ifndef __SSENVIRONMENT_H__
#define __SSENVIRONMENT_H__

#include "System/Sound/SsTypes.h"
#include "System/Sound/SsResource.h"

////////////////////////////////////////////////////////////////////////////////
// SsEnvironment
class SsEnvironment
{
public:
	SsEnvironment();
	virtual ~SsEnvironment();
	
	void interpolate( SsEnvironment* pEnvA, SsEnvironment* pEnvB, BcF32 Amount );
	void setDefault();
	void clamp();
	
public:
	BcF32	Density_;			
	BcF32	Diffusion_;			
	BcF32	Gain_;				
	BcF32	GainHF_;			
	BcF32	GainLF_;			
	BcF32	DecayTime_;			
	BcF32	DecayHFRatio_;		
	BcF32	DecayLFRatio_;		
	BcF32	ReflectionsGain_;	
	BcF32	ReflectionsDelay_;	
	MaVec3d	ReflectionsPan_;	
	BcF32	LateReverbGain_;	
	BcF32	LateReverbDelay_;
	MaVec3d	LateReverbPan_;	
	BcF32	EchoTime_;
	BcF32	EchoDepth_;
	BcF32	ModulationTime_;
	BcF32	ModulationDepth_;
	BcF32	AirAbsorptionGainHF_;
	BcF32	HFReference_;
	BcF32	LFReference_;
	BcF32	RoomRolloffFactor_;
	BcF32	DecayHFLimit_;
};

#endif
