/**************************************************************************
 *
 * File:		SsCoreImplAL.h
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#ifndef __SSCOREIMPLAL_H__
#define __SSCOREIMPLAL_H__

#include "SsCore.h"

#include "SsAL.h"
#include "SsChannelAL.h"

#include "BcScopedLock.h"
#include "BcMutex.h"

#include "SsEnvironment.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class SsCoreImplAL;

//////////////////////////////////////////////////////////////////////////
// Defines
#define MAX_AL_CHANNELS				( 32 )
#define MAX_AL_STEREO_SOURCES		( 2 )
#define MAX_AL_MONO_SOURCES			( MAX_AL_CHANNELS + ( 2 * MAX_AL_STEREO_SOURCES ) )

///////////////////////////////////////////////////////////////////////////
// SsCoreImplAL
class SsCoreImplAL:
	public SsCore
{
private:
	BcBool initEFX();

public:
	virtual void open();
	void open_threaded();

	virtual void update();
	void update_threaded();

	virtual void close();
	void close_threaded();


	BcBool isEFXEnabled() const;
	ALuint getALReverbAuxSlot() const;

	
public:
	virtual SsSample* createSample( BcU32 SampleRate, BcU32 Channels, BcBool Looping, void* pData, BcU32 DataSize );
	virtual void updateResource( SsResource* pResource );
	virtual void destroyResource( SsResource* pResource );
	
private:
	void createResource( SsResource* pResource );

public:
	virtual SsChannel* play( SsSample* pSample, SsChannelCallback* pCallback = NULL );
	virtual void setListener( const BcVec3d& Position, const BcVec3d& LookAt, const BcVec3d& Up );

public: // NEILO HACK.
	virtual void setEnvironment( const SsEnvironment& Environment );

public:
	SsChannelAL* allocChannel();
	void freeChannel( SsChannelAL* pSound );

private:
	//
	const BcChar*			pSelectedDevice_;
	ALCcontext*				ALContext_;
	ALCdevice*				ALDevice_;
	BcU32					InternalResourceCount_;

	BcBool					bEFXEnabled_;

	// Sound.
	BcU32					ChannelCount_;
	
	typedef std::list< SsChannelAL* > TChannelList;
	typedef TChannelList::iterator TChannelListIterator;
	
	TChannelList			FreeChannels_;
	TChannelList			UsedChannels_;
	
	// Listener.
	BcVec3d					ListenerPosition_;
	BcVec3d					ListenerLookAt_;
	BcVec3d					ListenerUp_;

	// Environment.
	SsEnvironment			Environment_;
	SsEnvironment			TargetEnvironment_;

	ALuint					ALReverbEffectSlot_;
	ALuint					ALReverbEffect_;

	BcMutex					ChannelLock_;

};

#endif
