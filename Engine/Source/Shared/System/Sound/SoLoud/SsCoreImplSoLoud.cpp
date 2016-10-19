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
#include "System/Os/OsCore.h"
#include "System/SysKernel.h"

#include <soloud.h>
#include <soloud_biquadresonantfilter.h>
#include <soloud_file.h>
#include <soloud_sfxr.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>

#include <algorithm>

//////////////////////////////////////////////////////////////////////////
// File IO.
class SsFileSoLoud : public SoLoud::File
{
public:
	SsFileSoLoud( const char* FileName )
	{
		File_.open( FileName, fsFM_READ );
		Position_ = 0;
	}

	virtual ~SsFileSoLoud()
	{
		File_.close();
	}

	int eof() override
	{
		return Position_ >= File_.size();
	}

	unsigned int read( unsigned char *aDst, unsigned int aBytes ) override
	{
		File_.read( Position_, aDst, aBytes );
		Position_ += aBytes;
		// TODO: Return bytes read.
		return aBytes;
	}
	
	unsigned int length() override
	{
		return static_cast< unsigned int >( File_.size() );
	}

	void seek( int aOffset ) override
	{
		Position_ = aOffset;
	}

	unsigned int pos() override
	{
		return Position_;
	}
	
	FILE * getFilePtr() override { return 0; }
	unsigned char * getMemPtr() override { return 0; }

private:
	FsFile File_;
	unsigned int Position_;
};

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
	if( GCommandLine_.hasArg( '\0', "nosoundthread" ) )
	{
		SsCore::JOB_QUEUE_ID = -1;
	}
	else
	{
		// Create our job queue.
		// - 1 thread if we have 4 or more hardware threads.
		// Synchronisation issues. Revisit post-hectic game jam.
		//SsCore::JOB_QUEUE_ID = SysKernel::pImpl()->createJobQueue( 1, 4 );
	}
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
		BcU32 Flags = 
			SoLoud::Soloud::CLIP_ROUNDOFF |
		SoLoud::Soloud::LEFT_HANDED_3D;
#if defined( PLATFORM_WINDOWS )
		SoLoud::result Result = 0;

		// Attempt to init WASAPI first.
		Result = SoLoudCore_->init(
			Flags,								// Flags.
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
			Flags,								// Flags.
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
			Flags,								// Flags.
			SoLoud::Soloud::AUTO,				// Backend.
			SoLoud::Soloud::AUTO,				// Sample rate.
			SoLoud::Soloud::AUTO );				// Buffer size.

		// Enable visualisation.
		SoLoudCore_->setVisualizationEnable( true );

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
	BcAssert( SoLoudCore_ != nullptr );

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
	BcAssert( SoLoudCore_ != nullptr );

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
	BcAssert( SoLoudCore_ != nullptr );

	auto* Resource = new SsSource( Params );

	auto createFunc = [ this, Resource, FileData ]()
	{
		auto InFileName = 
			*CsPaths::PACKED_CONTENT +
			std::string( "/" ) +
			FileData->FileHash_.getName() + std::string( ".dat" );
		SoLoud::AudioSource* AudioSource = nullptr;
		switch( FileData->Type_ )
		{
		case SsSourceFileData::SFXR:
			{
				auto SoLoudSfxr = new SoLoud::Sfxr();
				SoLoudSfxr->loadParamsFile( new SsFileSoLoud( InFileName.c_str() ) );
				AudioSource = SoLoudSfxr;
			}
			break;
		case SsSourceFileData::WAV:
			{
				auto SoLoudWav = new SoLoud::Wav();
				SoLoudWav->loadFile( new SsFileSoLoud( InFileName.c_str() ) );
				AudioSource = SoLoudWav;
			}
			break;
		case SsSourceFileData::WAVSTREAM:
			{
				auto SoLoudWav = new SoLoud::WavStream();
				SoLoudWav->loadFile( new SsFileSoLoud( InFileName.c_str() ) );

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
	BcAssert( SoLoudCore_ != nullptr );

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
	BcAssert( SoLoudCore_ != nullptr );

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
	BcAssert( SoLoudCore_ != nullptr );

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
		const SsChannelParams& InParams,
		SsChannelCallback DoneCallback ) 
{
	BcAssert( SoLoudCore_ != nullptr );

	SsChannelParams Params = InParams;

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

			// Setuo 3d parameters.
			AudioSource->set3dMinMaxDistance(
				Params.Min_,
				Params.Max_ );
			AudioSource->set3dAttenuation(
				static_cast< int >( Params.AttenuationModel_ ),
				Params.RolloffFactor_ );
			AudioSource->setVolume( Params.Gain_ );
			
			// TODO: Wait until SoLoud supports setting pitch + doppler.
			auto Position = Params.Position_;
			auto Velocity = MaVec3d( 0.0f, 0.0f, 0.0f ); //Params.Velocity_;

			// Play sound.
			SoLoud::handle Handle = SoLoudCore_->play3d( 
				*AudioSource,
				Position.x(), Position.y(), Position.z(),
				Velocity.x(), Velocity.y(), Velocity.z(),
				Params.Gain_,
				true, 
				0 );
			SoLoudCore_->setRelativePlaySpeed(
				Handle,
				Params.Pitch_ );
			// NOTE: Should be updated by play3d, or 3d channels shouldn't
			//       play until after they've been updated for the first time.
			//       Could move the unpause until after this call in internalUpdate
			//       but it may still be a bug with SoLoud that should be fixed.
			SoLoudCore_->update3dAudio();

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

	PSY_LOG( "WARNING: SsCoreImplSoLoud: Out of channels.\n" );
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// stopChannel
void SsCoreImplSoLoud::stopChannel( 
		SsChannel* Channel,
		BcBool ForceFlush )
{
	BcAssert( SoLoudCore_ != nullptr );

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
		}
		FlushFence.decrement();
	};

	// Increment and push stop channel.
	FlushFence.increment();
	SysKernel::pImpl()->pushFunctionJob( JOB_QUEUE_ID, stopChannelFunc );

	// If we're flushing, wait.
	FlushFence.wait();
}

//////////////////////////////////////////////////////////////////////////
// stopAllChannels
void SsCoreImplSoLoud::stopAllChannels( BcBool ForceFlush )
{

	SysFence FlushFence;

	// Stop source func.
	auto stopChannelFunc = [ this, ForceFlush, &FlushFence ]()
	{
		SoLoudCore_->stopAll();
		internalUpdate();
		FlushFence.decrement();
	};

	// Increment and push stop channel.
	FlushFence.increment();
	SysKernel::pImpl()->pushFunctionJob( JOB_QUEUE_ID, stopChannelFunc );

	// If we're flushing, wait.
	FlushFence.wait();
}

//////////////////////////////////////////////////////////////////////////
// updateChannel
void SsCoreImplSoLoud::updateChannel(
		SsChannel* Channel,
		const SsChannelParams& Params )
{
	BcAssert( SoLoudCore_ != nullptr );

	setChannelParams( Channel, Params );

	// Update source func.
	auto updateChannelFunc = [ this, Channel, Params ]()
	{
		SoLoud::handle Handle = Channel->getHandle< SoLoud::handle >();

		// TODO: Wait until SoLoud supports setting pitch + doppler.
		auto Position = Params.Position_;
		auto Velocity = MaVec3d( 0.0f, 0.0f, 0.0f ); //Params.Velocity_;

		SoLoudCore_->set3dSourceMinMaxDistance(
			Handle,
			Params.Min_,
			Params.Max_ );
		SoLoudCore_->set3dSourceAttenuation(
			Handle,
			static_cast< int >( Params.AttenuationModel_ ),
			Params.RolloffFactor_ );
		SoLoudCore_->set3dSourceParameters( 
			Handle,
			Position.x(), Position.y(), Position.z(),
			Velocity.x(), Velocity.y(), Velocity.z() );
		SoLoudCore_->setRelativePlaySpeed(
			Handle,
			Params.Pitch_ );
	};

	SysKernel::pImpl()->pushFunctionJob( JOB_QUEUE_ID, updateChannelFunc );
}

//////////////////////////////////////////////////////////////////////////
// setListener
void SsCoreImplSoLoud::setListener( const MaMat4d& Transform, const MaVec3d& Velocity )
{
	BcAssert( SoLoudCore_ != nullptr );

	// Set listener func.
	auto setListenerFunc = [ this, Transform, Velocity ]()
	{
		auto Position = Transform.translation();
		auto At = Transform.row2();
		auto Up = Transform.row1();

		SoLoudCore_->set3dListenerPosition( 
			Position.x(), Position.y(), Position.z() );
		SoLoudCore_->set3dListenerAt( 
			At.x(), At.y(), At.z() );
		SoLoudCore_->set3dListenerUp( 
			Up.x(), Up.y(), Up.z() );
		// TODO: Wait until SoLoud supports setting pitch + doppler.
#if 0
		SoLoudCore_->set3dListenerVelocity( 
			Velocity.x(), Velocity.y(), Velocity.z() );
#endif
	};

	SysKernel::pImpl()->pushFunctionJob( JOB_QUEUE_ID, setListenerFunc );
}

//////////////////////////////////////////////////////////////////////////
// getVisualisationData
void SsCoreImplSoLoud::getVisualisationData( 
	std::vector< BcF32 >& OutFFT, std::vector< BcF32 >& OutWave )
{
	BcAssert( SoLoudCore_ != nullptr );

	const size_t NoofElements = 256;
	OutFFT.assign( NoofElements, 0.0f );
	OutWave.assign( NoofElements, 0.0f );
	const auto* VisFFT = SoLoudCore_->calcFFT();
	const auto* VisWave = SoLoudCore_->getWave();
	BcAssert( VisFFT );
	BcAssert( VisWave );
	for( size_t Idx = 0; Idx < NoofElements; ++Idx )
	{
		OutFFT[ Idx ] = VisFFT[ Idx ];
		OutWave[ Idx ] = VisWave[ Idx ];
	}
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
	// Check focus and set volume accordingly.
	auto IsFocused = OsCore::pImpl()->getClient( 0 )->isFocused();
	SoLoudCore_->setGlobalVolume( IsFocused ? 1.0f : 0.0f );

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
	// TODO: Move to game thread.
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

