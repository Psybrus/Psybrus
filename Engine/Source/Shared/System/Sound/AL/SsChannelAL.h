/**************************************************************************
 *
 * File:		SsChannelAL.h
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#ifndef __SSCHANNELAL_H__
#define __SSCHANNELAL_H__

#include "System/Sound/SsChannel.h"

#include "System/Sound/AL/SsAL.h"
#include "System/Sound/AL/SsSampleAL.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class SsCoreImplAL;

//////////////////////////////////////////////////////////////////////////
// exSsChannelAL
class SsChannelAL:
	public SsChannel
{
public:
	SsChannelAL( SsCoreImplAL* Parent );
	virtual ~SsChannelAL();

	void				stop( BcBool ReleaseCallback = BcFalse );
	void				queue( SsSample* Sample );
	void				unqueue();
	BcU32				samplesQueued();
	BcU32				samplesComplete();
	void				play( SsSampleAL* Sample, SsChannelCallback* Callback );
	void				update();
	void				updateParams();
	
private:
	// AL stuff.
	ALuint ALSource_;
	ALuint ALFilter_;

	// State.
	SsChannelState State_;

	// Sample to play back.
	SsSample* Sample_;

	// Parent sound core.
	SsCoreImplAL* Parent_;
};



#endif
