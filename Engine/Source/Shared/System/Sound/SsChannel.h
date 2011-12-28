/**************************************************************************
 *
 * File:		SsSound.h
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#ifndef __SSSOUND_H__
#define __SSSOUND_H__

#include "SsTypes.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class SsChannel;

//////////////////////////////////////////////////////////////////////////
// SsChannelCallback
class SsChannelCallback
{
public:
	virtual void onStarted( SsChannel* pSound ){};
	virtual void onPlaying( SsChannel* pSound ){};
	virtual void onEnded( SsChannel* pSound ){};
};

//////////////////////////////////////////////////////////////////////////
// SsChannelState
enum SsChannelState
{
	ssCS_IDLE = 0,
	ssCS_PREPARED,
	ssCS_PLAYING,
	ssCS_PAUSED,
	ssCS_STOPPED
};


//////////////////////////////////////////////////////////////////////////
// SsChannel
class SsChannel
{
public:
	SsChannel();
	virtual ~SsChannel();

	SsChannelCallback* getCallback();

	void gain( BcF32 Position );
	BcF32 gain() const;

	void pitch( BcF32 Pitch );
	BcF32 pitch() const;

	void minDistance( BcF32 MinDistance );
	BcF32 minDistance() const;

	void rolloffFactor( BcF32 RolloffFactor );
	BcF32 rolloffFactor() const;

	void position( const BcVec3d& Position );
	const BcVec3d& position() const;

	virtual void stop( BcBool ReleaseCallback = BcFalse ) = 0;

protected:
	SsChannelCallback*	pCallback_;

	BcF32				Gain_;
	BcF32				Pitch_;
	BcF32				MinDistance_;
	BcF32				RolloffFactor_;
	BcVec3d				Position_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline SsChannel::SsChannel():
	pCallback_( NULL ),
	Gain_( 1.0f ),
	Pitch_( 1.0f ),
	MinDistance_( 1.0f ),
	RolloffFactor_( 0.05f ),
	Position_( BcVec3d( 0.0f, 0.0f, 0.0f ) )
{

}

inline SsChannel::~SsChannel()
{

}

inline SsChannelCallback* SsChannel::getCallback()
{
	return pCallback_;
}

inline void SsChannel::gain( BcF32 Gain )
{
	Gain_ = Gain;
}

inline BcF32 SsChannel::gain() const
{
	return Gain_;
}

inline void SsChannel::pitch( BcF32 Pitch )
{
	Pitch_ = Pitch;
}

inline BcF32 SsChannel::pitch() const
{
	return Pitch_;
}

inline void SsChannel::minDistance( BcF32 MinDistance )
{
	MinDistance_ = MinDistance;
}

inline BcF32 SsChannel::minDistance() const
{
	return MinDistance_;
}

inline void SsChannel::rolloffFactor( BcF32 RolloffFactor )
{
	RolloffFactor_ = RolloffFactor;
}

inline BcF32 SsChannel::rolloffFactor() const
{
	return RolloffFactor_;
}

inline void SsChannel::position( const BcVec3d& Position )
{
	Position_ = Position;
}

inline const BcVec3d& SsChannel::position() const
{
	return Position_;
}


#endif