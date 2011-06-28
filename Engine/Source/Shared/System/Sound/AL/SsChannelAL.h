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

#include "SsChannel.h"

#include "SsAL.h"
#include "SsSampleAL.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class SsCoreALInternal;

//////////////////////////////////////////////////////////////////////////
// exSsChannelAL
class SsChannelAL:
	public SsChannel
{
public:
	SsChannelAL( SsCoreALInternal* Parent );
	virtual ~SsChannelAL();

	void				stop();
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
	SsCoreALInternal* Parent_;
};



#endif
