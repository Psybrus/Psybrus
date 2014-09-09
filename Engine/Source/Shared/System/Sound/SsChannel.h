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
// SsChannelParams
class SsChannelParams
{
public:
	SsChannelParams();
	SsChannelParams( 
		MaVec3d Position, 
		BcF32 Gain = 1.0f,
		BcF32 Pitch = 1.0f );

	MaVec3d Position_;
	BcF32 Gain_;
	BcF32 Pitch_;
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
