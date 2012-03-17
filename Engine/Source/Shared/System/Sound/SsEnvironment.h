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

#include "SsTypes.h"
#include "SsResource.h"

////////////////////////////////////////////////////////////////////////////////
// SsEnvironment
class SsEnvironment
{
public:
	SsEnvironment();
	virtual ~SsEnvironment();
	
	void interpolate( SsEnvironment* pEnvA, SsEnvironment* pEnvB, BcReal Amount );
	void setDefault();
	void clamp();
	
public:
	BcReal	Density_;			
	BcReal	Diffusion_;			
	BcReal	Gain_;				
	BcReal	GainHF_;			
	BcReal	GainLF_;			
	BcReal	DecayTime_;			
	BcReal	DecayHFRatio_;		
	BcReal	DecayLFRatio_;		
	BcReal	ReflectionsGain_;	
	BcReal	ReflectionsDelay_;	
	BcVec3d	ReflectionsPan_;	
	BcReal	LateReverbGain_;	
	BcReal	LateReverbDelay_;
	BcVec3d	LateReverbPan_;	
	BcReal	EchoTime_;
	BcReal	EchoDepth_;
	BcReal	ModulationTime_;
	BcReal	ModulationDepth_;
	BcReal	AirAbsorptionGainHF_;
	BcReal	HFReference_;
	BcReal	LFReference_;
	BcReal	RoomRolloffFactor_;
	BcReal	DecayHFLimit_;
};

#endif