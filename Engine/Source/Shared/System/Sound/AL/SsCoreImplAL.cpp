/**************************************************************************
 *
 * File:		SsCoreImplAL.cpp
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#include "System/Sound/AL/SsCoreImplAL.h"

#include "System/SysKernel.h"

//////////////////////////////////////////////////////////////////////////
// Creator
SYS_CREATOR( SsCoreImplAL );

//////////////////////////////////////////////////////////////////////////
// initEFX
BcBool SsCoreImplAL::initEFX()
{
	BcBool bEFXSupport = BcFalse;

#if SS_AL_EFX_SUPPORTED
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
			bEFXSupport = BcTrue;
	}
#endif

	return bEFXSupport;
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void SsCoreImplAL::open()
{
	BcDelegate< void(*)() > Delegate( BcDelegate< void(*)() >::bind< SsCoreImplAL, &SsCoreImplAL::open_threaded >( this ) );
	SysKernel::pImpl()->enqueueDelegateJob( SsCore::WORKER_MASK, Delegate );

	// Wait for the render thread to complete.
	SysFence Fence;
	Fence.queue( SsCore::WORKER_MASK );
	Fence.wait();
}

//////////////////////////////////////////////////////////////////////////
// open_threaded
void SsCoreImplAL::open_threaded()
{
	// NOTE: Should enumerate devices and select an appropriate one here. For now we assume default.
	pSelectedDevice_ = NULL;
	ALContext_ = NULL;
	ALDevice_ = NULL;

	// Attempt to use generic software.
	pSelectedDevice_ = "Generic Software";

	// Open device
	ALDevice_ = alcOpenDevice( pSelectedDevice_ );
	
	BcPrintf( " - Opening device \"%s\"...\n", pSelectedDevice_ );

	// Fall back to default :(
	if( ALDevice_ == NULL )
	{
		BcPrintf( " - Failed to open device \"%s\", trying default.\n", pSelectedDevice_ );

		pSelectedDevice_ = NULL;
		ALDevice_ = alcOpenDevice( pSelectedDevice_ );
	}

	if( ALDevice_ != NULL )
	{
		// Create context for device.
		ALCint Attr[] = 
		{
			ALC_FREQUENCY, 44100,
			ALC_MONO_SOURCES, MAX_AL_MONO_SOURCES,
			ALC_STEREO_SOURCES, MAX_AL_STEREO_SOURCES,
			ALC_SYNC, SsCore::WORKER_MASK != 0x0 ? AL_TRUE : AL_FALSE,
			0	
		};

		ALContext_ = alcCreateContext( ALDevice_, &Attr[ 0 ] );
		alBreakOnError();
	}

	//
	if( ALContext_ != NULL )
	{
		BcPrintf( " - Context created.\n" );
		BcPrintf( " - Extensions: %s\n", alcGetString( ALDevice_, ALC_EXTENSIONS ) );

		// Make context current.
		alcMakeContextCurrent( ALContext_ );
		alBreakOnError();
		
		// Initialise EFX.
		bEFXEnabled_ = initEFX();

		// Determine how many channels to create.
		ALCint Sources;
		alcGetIntegerv( ALDevice_, ALC_MONO_SOURCES, 1, &Sources );
		ChannelCount_ = BcMin( BcU32( MAX_AL_CHANNELS ), BcU32( Sources ) );
		
		// Create channels.
		for( BcU32 i = 0; i < ChannelCount_; ++i )
		{
			FreeChannels_.push_back( new SsChannelAL( this ) );
		}

		BcPrintf( " - Created %u channels.\n", ChannelCount_ );
		
		//
		ListenerPosition_ = BcVec3d( 0.0f, 0.0f, 0.0f );
		ListenerLookAt_ = BcVec3d( 0.0f, 0.0f, 1.0f );
		ListenerUp_ = BcVec3d( 0.0f, 1.0f, 0.0f );

		if( bEFXEnabled_ == BcTrue )
		{
#if SS_AL_EFX_SUPPORTED
			BcPrintf( " - EFX is enabled.\n" );

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
void SsCoreImplAL::update()
{
	BcDelegate< void(*)() > Delegate( BcDelegate< void(*)() >::bind< SsCoreImplAL, &SsCoreImplAL::update_threaded >( this ) );
	SysKernel::pImpl()->enqueueDelegateJob( SsCore::WORKER_MASK, Delegate );
}

//////////////////////////////////////////////////////////////////////////
// update_threaded
//virtual
void SsCoreImplAL::update_threaded()
{
	SsChannelAL* pSound = NULL;

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
			-ListenerUp_.x(),
			-ListenerUp_.y(),
			-ListenerUp_.z()
		};

		alListenerfv( AL_POSITION, &Position[ 0 ] );
		alListenerfv( AL_ORIENTATION, &Orientation[ 0 ] );
	}
	
	// TODO: Command buffer this.
	// Update playing sounds.
	TChannelList UsedChannels = UsedChannels_; // Make a copy as the update can free channels.
	for( TChannelListIterator Iter( UsedChannels.begin() ); Iter != UsedChannels.end(); ++Iter )
	{
		pSound = (*Iter);	
		pSound->update();
	}

	// Interpolate environment.
	Environment_.interpolate( &Environment_, &TargetEnvironment_, 0.1f );

	static BcU32 EnvSetTimer = 0;
	if( ++EnvSetTimer >= 8 )
	{
		// Setup environment.
		alEffectf( ALReverbEffect_, AL_EAXREVERB_DENSITY, Environment_.Density_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_DIFFUSION, Environment_.Diffusion_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_GAIN, Environment_.Gain_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_GAINHF, Environment_.GainHF_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_GAINLF, Environment_.GainLF_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_DECAY_TIME, Environment_.DecayTime_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_DECAY_HFRATIO, Environment_.DecayHFRatio_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_DECAY_LFRATIO, Environment_.DecayLFRatio_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_REFLECTIONS_GAIN, Environment_.ReflectionsGain_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_REFLECTIONS_DELAY, Environment_.ReflectionsDelay_ );
		alEffectfv( ALReverbEffect_, AL_EAXREVERB_REFLECTIONS_PAN, (ALfloat*)&Environment_.ReflectionsPan_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_LATE_REVERB_GAIN, Environment_.LateReverbGain_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_LATE_REVERB_DELAY, Environment_.LateReverbDelay_ );
		alEffectfv( ALReverbEffect_, AL_EAXREVERB_LATE_REVERB_PAN, (ALfloat*)&Environment_.LateReverbPan_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_ECHO_TIME, Environment_.EchoTime_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_ECHO_DEPTH, Environment_.EchoDepth_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_MODULATION_TIME, Environment_.ModulationTime_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_MODULATION_DEPTH, Environment_.ModulationDepth_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, Environment_.AirAbsorptionGainHF_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_HFREFERENCE, Environment_.HFReference_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_LFREFERENCE, Environment_.LFReference_ );
		alEffectf( ALReverbEffect_, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, Environment_.RoomRolloffFactor_ );
		alEffecti( ALReverbEffect_, AL_EAXREVERB_DECAY_HFLIMIT, Environment_.DecayHFLimit_ > 0.5f ? AL_TRUE : AL_FALSE );
		alBreakOnError();

		alAuxiliaryEffectSloti( ALReverbEffectSlot_, AL_EFFECTSLOT_EFFECT, ALReverbEffect_ );
		alBreakOnError();

		EnvSetTimer = 0;
	}

	// Process context.	
	alcProcessContext( ALContext_ );
	alcSuspendContext( ALContext_ );
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void SsCoreImplAL::close()
{
	BcDelegate< void(*)() > Delegate( BcDelegate< void(*)() >::bind< SsCoreImplAL, &SsCoreImplAL::close_threaded >( this ) );
	SysKernel::pImpl()->enqueueDelegateJob( SsCore::WORKER_MASK, Delegate );

	// Wait for the render thread to complete.
	SysFence Fence;
	Fence.queue( SsCore::WORKER_MASK );
	Fence.wait();
}

//////////////////////////////////////////////////////////////////////////
// close_threaded
void SsCoreImplAL::close_threaded()
{
	BcAssert( InternalResourceCount_ == 0 );

 	BcAssertMsg( UsedChannels_.size() == 0, "SsCore ImplAL: All channels must be free." );
	
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
// isEFXEnabled
BcBool SsCoreImplAL::isEFXEnabled() const
{
	return bEFXEnabled_;
}

//////////////////////////////////////////////////////////////////////////
// getALReverbAuxSlot
ALuint SsCoreImplAL::getALReverbAuxSlot() const
{
	return ALReverbEffectSlot_;
}

//////////////////////////////////////////////////////////////////////////
// createSample
//virtual
SsSample* SsCoreImplAL::createSample( BcU32 SampleRate, BcU32 Channels, BcBool Looping, void* pData, BcU32 DataSize )
{
	SsSampleAL* pSample = new SsSampleAL( SampleRate, Channels, Looping, pData, DataSize );
	createResource( pSample );
	return pSample;
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void SsCoreImplAL::destroyResource( SsResource* pResource )
{
	pResource->preDestroy();

	SysResource::DestroyDelegate Delegate( SysResource::DestroyDelegate::bind< SysResource, &SysResource::destroy >( pResource ) );
	SysKernel::pImpl()->enqueueDelegateJob( SsCore::WORKER_MASK, Delegate );
}

//////////////////////////////////////////////////////////////////////////
// updateResource
void SsCoreImplAL::updateResource( SsResource* pResource )
{
	SysResource::UpdateDelegate Delegate( SysResource::UpdateDelegate::bind< SysResource, &SysResource::update >( pResource ) );
	SysKernel::pImpl()->enqueueDelegateJob( SsCore::WORKER_MASK, Delegate );
}

//////////////////////////////////////////////////////////////////////////
// createResource

void SsCoreImplAL::createResource( SsResource* pResource )
{
	SysResource::CreateDelegate Delegate( SysResource::CreateDelegate::bind< SysResource, &SysResource::create >( pResource ) );
	SysKernel::pImpl()->enqueueDelegateJob( SsCore::WORKER_MASK, Delegate );
}

//////////////////////////////////////////////////////////////////////////
// play
//virtual
SsChannel* SsCoreImplAL::play( SsSample* pSample, SsChannelCallback* pCallback )
{
	SsChannelAL* pChannel = allocChannel();

	// If we've allocated a channel, it's now over to the channel to do the rest.
	if( pChannel != NULL )
	{
		typedef BcDelegate< void(*)(SsSampleAL*, SsChannelCallback*) >  PlayDelegate;
		PlayDelegate Delegate( PlayDelegate::bind< SsChannelAL, &SsChannelAL::play >( pChannel ) );
		SysKernel::pImpl()->enqueueDelegateJob( SsCore::WORKER_MASK, Delegate, static_cast< SsSampleAL* >( pSample ), pCallback );
	}
	
	return pChannel;
}

//////////////////////////////////////////////////////////////////////////
// queue
//virtual
SsChannel* SsCoreImplAL::queue( SsSample* pSample, SsChannelCallback* pCallback )
{
	BcBreakpoint;
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// unregister
//virtual
void SsCoreImplAL::unregister( SsChannelCallback* Callback )
{
	for( TChannelListIterator Iter( UsedChannels_.begin() ); Iter != UsedChannels_.end(); ++Iter )
	{
		if( (*Iter)->getCallback() == Callback )
		{
			(*Iter)->setCallback( NULL );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setListener
//virtual
void SsCoreImplAL::setListener( const BcVec3d& Position, const BcVec3d& LookAt, const BcVec3d& Up )
{
	ListenerPosition_ = Position;
	ListenerLookAt_ = LookAt;
	ListenerUp_ = Up;
}

//////////////////////////////////////////////////////////////////////////
// setEnvironment
//virtual
void SsCoreImplAL::setEnvironment( const SsEnvironment& Environment )
{
	TargetEnvironment_ = Environment;
}

//////////////////////////////////////////////////////////////////////////
// allocChannel
SsChannelAL* SsCoreImplAL::allocChannel()
{
	BcScopedLock< BcMutex > Lock( ChannelLock_ ); // TODO: Lockless queue? Avoid this crap.

	SsChannelAL* pChannel = NULL;

	// Grab a free channel if we have any.
	if( FreeChannels_.size() > 0 )
	{
		pChannel = FreeChannels_.front();
		pChannel->setDefaults();
		FreeChannels_.pop_front();
		
		UsedChannels_.push_back( pChannel );
	}
	
	return pChannel;
}

//////////////////////////////////////////////////////////////////////////
// freeChannel
void SsCoreImplAL::freeChannel( SsChannelAL* pSound )
{
	BcScopedLock< BcMutex > Lock( ChannelLock_ ); // TODO: Lockless queue? Avoid this crap.

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

