/**************************************************************************
 *
 * File:		SsCoreImplSoLoud.cpp
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#include "System/Sound/SoLoud/SsCoreImplSoLoud.h"
#include "System/Content/CsCore.h"
#include "System/SysKernel.h"

#include <soloud.h>
#include <soloud_biquadresonantfilter.h>
#include <soloud_sfxr.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>

#include <algorithm>

//////////////////////////////////////////////////////////////////////////
// Creator
SYS_CREATOR( SsCoreImplSoLoud );

//////////////////////////////////////////////////////////////////////////
// Reflection.
REFLECTION_DEFINE_DERIVED( SsCoreImplSoLoud );

void SsCoreImplSoLoud::StaticRegisterClass()
{
	ReRegisterClass< SsCoreImplSoLoud, Super >();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
SsCoreImplSoLoud::SsCoreImplSoLoud():
	SoLoudCore_( nullptr )
{
	// Create our job queue.
	// - 1 thread if we have 4 or more hardware threads.
	// Synchronisation issues. Revisit post-hectic game jam.
	//SsCore::JOB_QUEUE_ID = SysKernel::pImpl()->createJobQueue( 1, 4 );

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
SsCoreImplSoLoud::~SsCoreImplSoLoud()
{

}

//////////////////////////////////////////////////////////////////////////
// open
void SsCoreImplSoLoud::open()
{
	// Create SoLoud interface.
	SoLoudCore_ = new SoLoud::Soloud();

	// Create channels.
	SsChannelParams DefaultChannelParams;
	for( BcU32 Idx = 0; Idx < VOICE_COUNT; ++Idx )
	{
		FreeChannels_.push_back( new SsChannel( DefaultChannelParams ) );
	}

	// Open func to run on worker.
	auto openFunc = [ this ]()
	{
#if defined( PLATFORM_WINDOWS )
		SoLoud::result Result = 0;

		// Attempt to init WASAPI first.
		Result = SoLoudCore_->init(
			SoLoud::Soloud::CLIP_ROUNDOFF,		// Flags.
			SoLoud::Soloud::WASAPI,				// Backend.
			SoLoud::Soloud::AUTO,				// Sample rate.
			SoLoud::Soloud::AUTO );				// Buffer size.
		
		if( Result == 0 )
		{
			WaitFence_.decrement();
			return;
		}

		// Fall back to WIMM.
		Result = SoLoudCore_->init(
			SoLoud::Soloud::CLIP_ROUNDOFF,		// Flags.
			SoLoud::Soloud::WINMM,				// Backend.
			SoLoud::Soloud::AUTO,				// Sample rate.
			SoLoud::Soloud::AUTO );				// Buffer size.

		if( Result == 0 )
		{
			WaitFence_.decrement();
			return;
		}

#endif

		// Attempt auto backend.
		SoLoudCore_->init(
			SoLoud::Soloud::CLIP_ROUNDOFF,		// Flags.
			SoLoud::Soloud::AUTO,				// Backend.
			SoLoud::Soloud::AUTO,				// Sample rate.
			SoLoud::Soloud::AUTO );				// Buffer size.

		WaitFence_.decrement();
		
	};

	WaitFence_.increment();
	SysKernel::pImpl()->pushFunctionJob( JOB_QUEUE_ID, openFunc );
	WaitFence_.wait();
}

//////////////////////////////////////////////////////////////////////////
// update
void SsCoreImplSoLoud::update()
{
	auto updateFunc = [ this ]()
	{
		internalUpdate();
	};

	SysKernel::pImpl()->pushFunctionJob( JOB_QUEUE_ID, updateFunc );
}

//////////////////////////////////////////////////////////////////////////
// close
void SsCoreImplSoLoud::close()
{
	auto closeFunc = [ this ]()
	{
		// Stop all sounds.
		SoLoudCore_->stopAll();

		// Loop over until we have stopped all sounds.
		while( UsedChannels_.size() > 0 && PendingChannels_.size() > 0 )
		{
			internalUpdate();
		}

		SoLoudCore_->deinit();
		WaitFence_.decrement();
	};

	WaitFence_.increment();
	SysKernel::pImpl()->pushFunctionJob( JOB_QUEUE_ID, closeFunc );
	WaitFence_.wait();

	// Free channels.
	std::for_each( FreeChannels_.begin(), FreeChannels_.end(), 
		[]( SsChannel* Channel )
		{
			delete Channel; 
		} );
	std::for_each( UsedChannels_.begin(), UsedChannels_.end(), 
		[]( SsChannel* Channel )
		{ 
			delete Channel;
		} );

	// Clear lists.
	FreeChannels_.clear();
	UsedChannels_.clear();

	// Free SoLoud core.
	delete SoLoudCore_;
	SoLoudCore_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// createBus
SsBus* SsCoreImplSoLoud::createBus( const SsBusParams& Params )
{
	auto* Resource = new SsBus( Params );

	auto createFunc = [ this, Resource ]()
	{
		auto SoLoudBus = new SoLoud::Bus();
		Resource->setHandle( SoLoudBus );
	};

	SysKernel::pImpl()->pushFunctionJob( JOB_QUEUE_ID, createFunc );

	return Resource;
}

//////////////////////////////////////////////////////////////////////////
// createFilter
SsFilter* SsCoreImplSoLoud::createFilter( const SsFilterParams& Params )
{
	auto* Resource = new SsFilter( Params );

	auto createFunc = [ this, Resource ]()
	{
		// TODO: Configure type of filter.
		auto SoLoudFilter = new SoLoud::BiquadResonantFilter();
		Resource->setHandle( SoLoudFilter );
	};

	SysKernel::pImpl()->pushFunctionJob( JOB_QUEUE_ID, createFunc );

	return Resource;
}

//////////////////////////////////////////////////////////////////////////
// createSource
class SsSource* SsCoreImplSoLoud::createSource(
	const SsSourceParams& Params,
	const SsSourceFileData* FileData )
{
	auto* Resource = new SsSource( Params );

	auto createFunc = [ this, Resource, FileData ]()
	{
		auto InFileName = 
			*CsCore::pImpl()->getPackagePackedPath( BcName::INVALID ) + 
			std::string( "/" ) + 
			FileData->FileHash_.getName() + std::string( ".dat" );
		SoLoud::AudioSource* AudioSource = nullptr;
		switch( FileData->Type_ )
		{
		case SsSourceFileData::SFXR:
			{
				auto SoLoudSfxr = new SoLoud::Sfxr();
				SoLoudSfxr->loadParams( InFileName.c_str() );
				AudioSource = SoLoudSfxr;
			}
			break;
		case SsSourceFileData::WAV:
			{
				auto SoLoudWav = new SoLoud::Wav();
				SoLoudWav->load( InFileName.c_str() );
				AudioSource = SoLoudWav;
			}
			break;
		case SsSourceFileData::WAVSTREAM:
			{
				auto SoLoudWav = new SoLoud::WavStream();
				SoLoudWav->load( InFileName.c_str() );
				AudioSource = SoLoudWav;
			}
			break;
		case SsSourceFileData::MODPLUG:
			BcBreakpoint;
			break;
		}
		AudioSource->setLooping( FileData->Looping_ ? true : false );
		Resource->setHandle( AudioSource );
	};

	SysKernel::pImpl()->pushFunctionJob( JOB_QUEUE_ID, createFunc );

	return Resource;
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void SsCoreImplSoLoud::destroyResource( SsBus* Resource )
{
	auto deleteFunc = [ this, Resource ]()
	{
		auto SoLoudBus = Resource->getHandle< SoLoud::Bus* >();
		SoLoudCore_->stopAudioSource( *SoLoudBus );
		delete SoLoudBus;
		delete Resource;
	};

	SysKernel::pImpl()->pushFunctionJob( JOB_QUEUE_ID, deleteFunc );
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void SsCoreImplSoLoud::destroyResource( SsFilter* Resource )
{
	auto deleteFunc = [ this, Resource ]()
	{
		auto SoLoudFilter = Resource->getHandle< SoLoud::Filter* >();
		delete SoLoudFilter;
		delete Resource;
	};

	SysKernel::pImpl()->pushFunctionJob( JOB_QUEUE_ID, deleteFunc );
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void SsCoreImplSoLoud::destroyResource( SsSource* Resource )
{
	auto deleteFunc = [ this, Resource ]()
	{
		auto SoLoudAudioSource = Resource->getHandle< SoLoud::AudioSource* >();
		SoLoudCore_->stopAudioSource( *SoLoudAudioSource );
		delete SoLoudAudioSource;
		delete Resource;
	};

	SysKernel::pImpl()->pushFunctionJob( JOB_QUEUE_ID, deleteFunc );
}

//////////////////////////////////////////////////////////////////////////
// playSource
SsChannel* SsCoreImplSoLoud::playSource( 
		SsSource* Source,
		const SsChannelParams& Params,
		SsChannelCallback DoneCallback ) 
{
	// Allocate channel.
	if( auto* Channel = allocChannel() )
	{
		// Store done callback.
		if( DoneCallback != nullptr )
		{
			std::lock_guard< std::recursive_mutex > Lock( ChannelMutex_ );
			BcAssert( ChannelDoneCallbacks_.find( Channel ) == ChannelDoneCallbacks_.end() );
			ChannelDoneCallbacks_[ Channel ] = std::move( DoneCallback );
		}

		// Set channel's params.
		setChannelParams( Channel, Params );

		// Play source func.
		auto playSourceFunc = [ this, Channel, Source, Params ]()
		{

			SoLoud::AudioSource* AudioSource = Source->getHandle< SoLoud::AudioSource* >();
			SoLoud::handle Handle = SoLoudCore_->play3d( 
				*AudioSource,
				Params.Position_.x(), Params.Position_.y(), Params.Position_.z(),
				Params.Velocity_.x(), Params.Velocity_.y(), Params.Velocity_.z(),
				Params.Gain_,
				true, 
				0 );
			SoLoudCore_->setRelativePlaySpeed(
				Handle,
				Params.Pitch_ );
			SoLoudCore_->setPause(
				Handle,
				false );
			updateChannel( Channel, Handle ); 
		};

		SysKernel::pImpl()->pushFunctionJob( JOB_QUEUE_ID, playSourceFunc );

		// Update params.
		updateChannel( Channel, Channel->getParams() );

		return Channel;
	}

	BcPrintf( "WARNING: SsCoreImplSoLoud: Out of channels.\n" );
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// stopChannel
void SsCoreImplSoLoud::stopChannel( 
		SsChannel* Channel,
		BcBool ForceFlush )
{
	SysFence FlushFence;

	// Stop source func.
	auto stopChannelFunc = [ this, Channel, ForceFlush, &FlushFence ]()
	{
		SoLoud::handle Handle = Channel->getHandle< SoLoud::handle >();
		SoLoudCore_->stop( Handle );

		// TODO: Remove from used queue at this point.
		//       No need to force updated until voice is invalid then.

		// Update until the sound has stopped.
		if( ForceFlush )
		{
			do 
			{
				internalUpdate();
			}
			while( SoLoudCore_->isValidVoiceHandle( Handle ) );
			FlushFence.decrement();
		}
	};

	// Increment and push stop channel.
	FlushFence.increment();
	SysKernel::pImpl()->pushFunctionJob( JOB_QUEUE_ID, stopChannelFunc );

	// If we're flushing, wait.
	if( ForceFlush )
	{
		FlushFence.wait();
	}
}

//////////////////////////////////////////////////////////////////////////
// updateChannel
void SsCoreImplSoLoud::updateChannel(
		SsChannel* Channel,
		const SsChannelParams& Params )
{
	// Update source func.
	auto updateChannelFunc = [ this, Channel, Params ]()
	{
		SoLoud::handle Handle = Channel->getHandle< SoLoud::handle >();

		SoLoudCore_->set3dSourceParameters( 
			Handle,
			Params.Position_.x(), Params.Position_.y(), Params.Position_.z(),
			Params.Velocity_.x(), Params.Velocity_.y(), Params.Velocity_.z() );
		SoLoudCore_->setVolume( 
			Handle,
			Params.Gain_ );
		SoLoudCore_->setRelativePlaySpeed(
			Handle,
			Params.Pitch_ );
	};

	SysKernel::pImpl()->pushFunctionJob( JOB_QUEUE_ID, updateChannelFunc );
}

//////////////////////////////////////////////////////////////////////////
// allocChannel
SsChannel* SsCoreImplSoLoud::allocChannel()
{
	std::lock_guard< std::recursive_mutex > Lock( ChannelMutex_ );
	SsChannel* Channel = nullptr;
	if( FreeChannels_.size() > 0 )
	{
		Channel = FreeChannels_.front();
		FreeChannels_.pop_front();
		PendingChannels_.push_back( Channel );
	}
	return Channel;
}

//////////////////////////////////////////////////////////////////////////
// updateChannel
void SsCoreImplSoLoud::updateChannel( SsChannel* Channel, SoLoud::handle Handle )
{
	if( Channel != nullptr )
	{
		std::lock_guard< std::recursive_mutex > Lock( ChannelMutex_ );
		Channel->setHandle( Handle );
		PendingChannels_.remove( Channel );
		UsedChannels_.push_back( Channel );
	}
}

//////////////////////////////////////////////////////////////////////////
// freeChannel
void SsCoreImplSoLoud::freeChannel( SsChannel* Channel )
{
	if( Channel != nullptr )
	{
		std::lock_guard< std::recursive_mutex > Lock( ChannelMutex_ );
		UsedChannels_.remove( Channel );
		FreeChannels_.push_back( Channel );
	}
}

//////////////////////////////////////////////////////////////////////////
// internalUpdate
void SsCoreImplSoLoud::internalUpdate()
{
	// Update 3D audio.
	SoLoudCore_->update3dAudio();

	// Update channels.
	std::lock_guard< std::recursive_mutex > Lock( ChannelMutex_ );
	std::vector< SsChannel* > ChannelsToFree;
	std::for_each( UsedChannels_.begin(), UsedChannels_.end(),
		[ this, &ChannelsToFree ]( SsChannel* Channel )
	{
		SoLoud::handle Handle = Channel->getHandle< SoLoud::handle >();
		if( !SoLoudCore_->isValidVoiceHandle( Handle ) )
		{
			ChannelsToFree.push_back( Channel );
		}
	} );

	// Free channels.
	std::for_each( ChannelsToFree.begin(), ChannelsToFree.end(),
		[ this ]( SsChannel* Channel )
	{
		auto DoneCallback = ChannelDoneCallbacks_.find( Channel );
		if( DoneCallback != ChannelDoneCallbacks_.end() )
		{
			DoneCallback->second( Channel );
			ChannelDoneCallbacks_.erase( DoneCallback );
		}
		freeChannel( Channel );
	} );
}
