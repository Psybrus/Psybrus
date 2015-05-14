/**************************************************************************
 *
 * File:		SsChannel.h
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#ifndef __SSCHANNEL_H__
#define __SSCHANNEL_H__

#include "System/Sound/SsResource.h"

//////////////////////////////////////////////////////////////////////////
// SsChannelState
enum class SsChannelState
{
	IDLE = 0,
	PREPARED,
	PLAYING,
	PAUSED,
	STOPPED
};

//////////////////////////////////////////////////////////////////////////
// SsAttenuationModel
enum SsAttenuationModel
{
	// No attenuation
	NONE = 0,
	// Inverse distance attenuation model
	INVERSE = 1,
	// Linear distance attenuation model
	LINEAR = 2,
	// Exponential distance attenuation model
	EXPONENTIAL = 3
};

//////////////////////////////////////////////////////////////////////////
// SsChannelParams
class SsChannelParams
{
public:
	SsChannelParams( 
		BcF32 Gain = 1.0f,
		BcF32 Pitch = 1.0f,
		BcF32 Min = 1.0f,
		BcF32 Max = 1000.0f,
		SsAttenuationModel AttenuationModel = SsAttenuationModel::EXPONENTIAL,
		BcF32 RolloffFactor = 1.0f,
		MaVec3d Position = MaVec3d( 0.0f, 0.0f, 0.0f ),
		MaVec3d Velocity = MaVec3d( 0.0f, 0.0f, 0.0f ) );

	BcF32 Gain_;
	BcF32 Pitch_;
	BcF32 Min_;
	BcF32 Max_;
	SsAttenuationModel AttenuationModel_;
	BcF32 RolloffFactor_;
	MaVec3d Position_;
	MaVec3d Velocity_;
};

//////////////////////////////////////////////////////////////////////////
// SsChannel
class SsChannel:
	public SsResource
{
public:
	SsChannel( const SsChannelParams& Params );
	virtual ~SsChannel();

	const SsChannelParams& getParams() const;

private:
	friend class SsCore;
	void setParams( const SsChannelParams& Params );

private:
	SsChannelParams Params_;
};

#endif
