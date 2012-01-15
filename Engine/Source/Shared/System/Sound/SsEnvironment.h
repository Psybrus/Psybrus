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
class SsEnvironment:
	public SsResource
{
public:
	SsEnvironment();
	virtual ~SsEnvironment();
	
	void interpolate( SsEnvironment* pEnvA, SsEnvironment* pEnvB, BcReal Amount );
	void setDefault();
	void clamp();
	
protected:
	BcReal	Density_;			
	BcReal	Diffusion_;			
	BcReal	Gain_;				
	BcReal	GainHF_;			
	BcReal	DecayTime_;			
	BcReal	DecayHFRatio_;		
	BcReal	ReflectionsGain_;	
	BcReal	ReflectionsDelay_;	
	BcReal	LateReverbGain_;	
	BcReal	LateReverbDelay_;
	BcReal	AirAbsorptionGainHF_;
	BcReal	RoomRolloffFactor_;
	BcBool	DecayHFLimit_;
};

#endif