/**************************************************************************
 *
 * File:		SsCoreALInternal.cpp
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#include "SsCoreALInternal.h"

//////////////////////////////////////////////////////////////////////////
// Creator
SYS_CREATOR( SsCoreALInternal );

//////////////////////////////////////////////////////////////////////////
// initEFX
BcBool SsCoreALInternal::initEFX()
{
	BcBool bEFXSupport = BcFalse;

#ifdef EX_AL_EFX
	if (alcIsExtensionPresent( ALDevice_, (ALCchar*)ALC_EXT_EFX_NAME ) )
	{
		// Get function pointers
		alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
		alDeleteEffects = (LPALDELETEEFFECTS )alGetProcAddress("alDeleteEffects");
		alIsEffect = (LPALISEFFECT )alGetProcAddress("alIsEffect");
		alEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
		alEffectiv = (LPALEFFECTIV)alGetProcAddress("alEffectiv");
		alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");
		alEffectfv = (LPALEFFECTFV)alGetProcAddress("alEffectfv");
		alGetEffecti = (LPALGETEFFECTI)alGetProcAddress("alGetEffecti");
		alGetEffectiv = (LPALGETEFFECTIV)alGetProcAddress("alGetEffectiv");
		alGetEffectf = (LPALGETEFFECTF)alGetProcAddress("alGetEffectf");
		alGetEffectfv = (LPALGETEFFECTFV)alGetProcAddress("alGetEffectfv");
		alGenFilters = (LPALGENFILTERS)alGetProcAddress("alGenFilters");
		alDeleteFilters = (LPALDELETEFILTERS)alGetProcAddress("alDeleteFilters");
		alIsFilter = (LPALISFILTER)alGetProcAddress("alIsFilter");
		alFilteri = (LPALFILTERI)alGetProcAddress("alFilteri");
		alFilteriv = (LPALFILTERIV)alGetProcAddress("alFilteriv");
		alFilterf = (LPALFILTERF)alGetProcAddress("alFilterf");
		alFilterfv = (LPALFILTERFV)alGetProcAddress("alFilterfv");
		alGetFilteri = (LPALGETFILTERI )alGetProcAddress("alGetFilteri");
		alGetFilteriv= (LPALGETFILTERIV )alGetProcAddress("alGetFilteriv");
		alGetFilterf = (LPALGETFILTERF )alGetProcAddress("alGetFilterf");
		alGetFilterfv= (LPALGETFILTERFV )alGetProcAddress("alGetFilterfv");
		alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
		alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
		alIsAuxiliaryEffectSlot = (LPALISAUXILIARYEFFECTSLOT)alGetProcAddress("alIsAuxiliaryEffectSlot");
		alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
		alAuxiliaryEffectSlotiv = (LPALAUXILIARYEFFECTSLOTIV)alGetProcAddress("alAuxiliaryEffectSlotiv");
		alAuxiliaryEffectSlotf = (LPALAUXILIARYEFFECTSLOTF)alGetProcAddress("alAuxiliaryEffectSlotf");
		alAuxiliaryEffectSlotfv = (LPALAUXILIARYEFFECTSLOTFV)alGetProcAddress("alAuxiliaryEffectSlotfv");
		alGetAuxiliaryEffectSloti = (LPALGETAUXILIARYEFFECTSLOTI)alGetProcAddress("alGetAuxiliaryEffectSloti");
		alGetAuxiliaryEffectSlotiv = (LPALGETAUXILIARYEFFECTSLOTIV)alGetProcAddress("alGetAuxiliaryEffectSlotiv");
		alGetAuxiliaryEffectSlotf = (LPALGETAUXILIARYEFFECTSLOTF)alGetProcAddress("alGetAuxiliaryEffectSlotf");
		alGetAuxiliaryEffectSlotfv = (LPALGETAUXILIARYEFFECTSLOTFV)alGetProcAddress("alGetAuxiliaryEffectSlotfv");

		if (alGenEffects &&	alDeleteEffects && alIsEffect && alEffecti && alEffectiv &&	alEffectf &&
			alEffectfv && alGetEffecti && alGetEffectiv && alGetEffectf && alGetEffectfv &&	alGenFilters &&
			alDeleteFilters && alIsFilter && alFilteri && alFilteriv &&	alFilterf && alFilterfv &&
			alGetFilteri &&	alGetFilteriv && alGetFilterf && alGetFilterfv && alGenAuxiliaryEffectSlots &&
			alDeleteAuxiliaryEffectSlots &&	alIsAuxiliaryEffectSlot && alAuxiliaryEffectSloti &&
			alAuxiliaryEffectSlotiv && alAuxiliaryEffectSlotf && alAuxiliaryEffectSlotfv &&
			alGetAuxiliaryEffectSloti && alGetAuxiliaryEffectSlotiv && alGetAuxiliaryEffectSlotf &&
			alGetAuxiliaryEffectSlotfv)
			bEFXSupport = exTrue;
	}
#endif

	return bEFXSupport;
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void SsCoreALInternal::open()
{
	// NOTE: Should enumerate devices and select an appropriate one here. For now we assume default.
	pSelectedDevice_ = NULL;
	ALContext_ = NULL;
	ALDevice_ = NULL;

	// Open device
	ALDevice_ = alcOpenDevice( pSelectedDevice_ );
	alBreakOnError();

	if( ALDevice_ != NULL )
	{
		// Create context for device.
		ALCint Attr[] = 
		{
			ALC_FREQUENCY, 44100,
			ALC_MONO_SOURCES, MAX_AL_MONO_SOURCES,
			ALC_STEREO_SOURCES, MAX_AL_STEREO_SOURCES,
			NULL	
		};

		ALContext_ = alcCreateContext( ALDevice_, &Attr[ 0 ] );
		alBreakOnError();
	}

	//
	if( ALContext_ != NULL )
	{
		// Make context current.
		alcMakeContextCurrent( ALContext_ );
		alBreakOnError();

		// Determine how many channels to create.
		ALCint Sources;
		alcGetIntegerv( ALDevice_, ALC_MONO_SOURCES, 1, &Sources );
		ChannelCount_ = BcMin( BcU32( MAX_AL_CHANNELS ), BcU32( Sources ) );
		
		// Create channels.
		for( BcU32 i = 0; i < ChannelCount_; ++i )
		{
			FreeChannels_.push_back( new SsChannelAL( this ) );
		}

		//
		ListenerPosition_ = BcVec3d( 0.0f, 0.0f, 0.0f );
		ListenerLookAt_ = BcVec3d( 0.0f, 0.0f, 1.0f );
		ListenerUp_ = BcVec3d( 0.0f, 1.0f, 0.0f );

		//
		bEFXEnabled_ = initEFX();

		if( bEFXEnabled_ == BcTrue )
		{
#ifdef EX_AL_EFX
			// Setup effect slot.
			alGenAuxiliaryEffectSlots( 1, &ALReverbEffectSlot_ );
			alGetError();

			// Setup effect.
			alGenEffects( 1, &ALReverbEffect_ );
			alBreakOnError();

			alEffecti( ALReverbEffect_, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB );
			alBreakOnError();
#endif
		}
	}

	InternalResourceCount_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void SsCoreALInternal::update()
{
	SsChannelAL* pSound = NULL;

	// Execute command buffer.
	CommandBuffer_.execute();

	// Update SetListener.
	{
		ALfloat Position[] =
		{
			ListenerPosition_.x(),
			ListenerPosition_.y(),
			ListenerPosition_.z()
		};

		ALfloat Orientation[] =
		{
			ListenerLookAt_.x(),
			ListenerLookAt_.y(),
			ListenerLookAt_.z(),
			ListenerUp_.x(),
			ListenerUp_.y(),
			ListenerUp_.z()
		};

		alListenerfv( AL_POSITION, &Position[ 0 ] );
		alListenerfv( AL_ORIENTATION, &Orientation[ 0 ] );
	}

	// TODO: Command buffer this.
	// Update playing sounds.
	TChannelList UsedChannels = UsedChannels_; // Make a copy as the update can free channbels.
	for( TChannelListIterator Iter( UsedChannels.begin() ); Iter != UsedChannels.end(); ++Iter )
	{
		pSound = (*Iter);	
		pSound->update();
	}

	// Process context.	
	alcProcessContext( ALContext_ );
	alcSuspendContext( ALContext_ );
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void SsCoreALInternal::close()
{
	BcAssert( InternalResourceCount_ == 0 );
	BcAssertMsg( UsedChannels_.size() == 0, "SsCoreALInternal: All channels must be free." );
	
	// Destroy channels.
	for( TChannelListIterator Iter( FreeChannels_.begin() ); Iter != FreeChannels_.end(); ++Iter )
	{
		delete (*Iter);
	}
	FreeChannels_.clear();
	
	// Destroy context and all that.
	alcMakeContextCurrent( NULL );
	alBreakOnError();

	alcDestroyContext( ALContext_ );
	alBreakOnError();

	alcCloseDevice( ALDevice_ );
	alBreakOnError();
}

//////////////////////////////////////////////////////////////////////////
// createSample
//virtual
SsSample* SsCoreALInternal::createSample( BcU32 SampleRate, BcU32 Channels, BcBool Looping, void* pData, BcU32 DataSize )
{
	SsSampleAL* pSample = new SsSampleAL( SampleRate, Channels, Looping, pData, DataSize );
	createResource( pSample );
	return pSample;
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void SsCoreALInternal::destroyResource( SsResource* pResource )
{
	pResource->preDestroy();

	BcDelegateCall< void(*)() > DelegateCall( BcDelegate< void(*)() >::bind< SsResource, &SsResource::destroy >( pResource ) );
	CommandBuffer_.enqueue( DelegateCall );
}

//////////////////////////////////////////////////////////////////////////
// updateResource
void SsCoreALInternal::updateResource( SsResource* pResource )
{
	BcDelegateCall< void(*)() > DelegateCall( BcDelegate< void(*)() >::bind< SsResource, &SsResource::update >( pResource ) );
	CommandBuffer_.enqueue( DelegateCall );
}

//////////////////////////////////////////////////////////////////////////
// createResource

void SsCoreALInternal::createResource( SsResource* pResource )
{
	BcDelegateCall< void(*)() > DelegateCall( BcDelegate< void(*)() >::bind< SsResource, &SsResource::create >( pResource ) );
	CommandBuffer_.enqueue( DelegateCall );
}

//////////////////////////////////////////////////////////////////////////
// play
//virtual
SsChannel* SsCoreALInternal::play( SsSample* pSample, SsChannelCallback* pCallback )
{
	SsChannelAL* pChannel = allocChannel();

	// If we've allocated a channel, it's now over to the channel to do the rest.
	if( pChannel != NULL )
	{
		BcDelegateCall< void(*)(SsSampleAL*, SsChannelCallback*) > DelegateCall( BcDelegate< void(*)(SsSampleAL*, SsChannelCallback*) >::bind< SsChannelAL, &SsChannelAL::play >( pChannel ) );
		CommandBuffer_.enqueue( DelegateCall, static_cast< SsSampleAL* >( pSample ), pCallback );
	}
	
	return pChannel;
}

//////////////////////////////////////////////////////////////////////////
// setListener
//virtual
void SsCoreALInternal::setListener( const BcVec3d& Position, const BcVec3d& LookAt, const BcVec3d& Up )
{
	ListenerPosition_ = Position;
	ListenerLookAt_ = LookAt;
	ListenerUp_ = Up;
}

//////////////////////////////////////////////////////////////////////////
// allocChannel
SsChannelAL* SsCoreALInternal::allocChannel()
{
	SsChannelAL* pChannel = NULL;

	// Grab a free channel if we have any.
	if( FreeChannels_.size() > 0 )
	{
		pChannel = FreeChannels_.front();
		FreeChannels_.pop_front();
		
		UsedChannels_.push_back( pChannel );
	}
	
	return pChannel;
}

//////////////////////////////////////////////////////////////////////////
// freeChannel
void SsCoreALInternal::freeChannel( SsChannelAL* pSound )
{
	// Remove from used list.
	for( TChannelListIterator Iter( UsedChannels_.begin() ); Iter != UsedChannels_.end(); ++Iter )
	{
		if( (*Iter) == pSound )
		{
			UsedChannels_.erase( Iter );
			break;
		}
	}

	// Push into free list.
	FreeChannels_.push_back( pSound );
}

