/**************************************************************************
 *
 * File:		SsChannelAL.cpp
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#include "System/Sound/AL/SsChannelAL.h"

#include "System/Sound/AL/SsCoreImplAL.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
SsChannelAL::SsChannelAL( SsCoreImplAL* Parent ):
	ALSource_( NULL ),
	ALFilter_( NULL ),
	State_( ssCS_IDLE ),
	Sample_( NULL ),
	Parent_( Parent )
{
	// Generate source.
	alGenSources( 1, &ALSource_ );
	alBreakOnError();

#if SS_AL_EFX_SUPPORTED
	if( SsCore::pImpl< SsCoreImplAL >()->isEFXEnabled() )
	{
		// Generate filter (if supported)
		alGenFilters( 1, &ALFilter_ );
		alBreakOnError();
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// Dtor
SsChannelAL::~SsChannelAL()
{
	// Delete source.
	alDeleteSources( 1, &ALSource_ );
	alBreakOnError();

#if SS_AL_EFX_SUPPORTED
	if( SsCore::pImpl< SsCoreImplAL >()->isEFXEnabled() )
	{
		// Delete filters.
		alDeleteFilters( 1, &ALFilter_ );
		alBreakOnError();
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// stop
//virtual
void SsChannelAL::stop( BcBool ReleaseCallback )
{
	// TODO: Lock/fence!!
	if( ReleaseCallback )
	{
		pCallback_ = NULL;
	}
	
	alSourceStop( ALSource_ );
}

//////////////////////////////////////////////////////////////////////////
// queue
void SsChannelAL::queue( SsSample* Sample )
{
	alSourcei( ALSource_, AL_LOOPING, BcFalse );
	ALuint Buffer = Sample->getHandle< ALuint >(); 
	alSourceQueueBuffers( ALSource_, 1, &Buffer );
}

//////////////////////////////////////////////////////////////////////////
// unqueue
void SsChannelAL::unqueue()
{
	ALuint Buffer; 
	alSourceUnqueueBuffers( ALSource_, 1, &Buffer );
}

//////////////////////////////////////////////////////////////////////////
// samplesQueued
BcU32 SsChannelAL::samplesQueued()
{
	ALint Queued;
	alGetSourcei( ALSource_, AL_BUFFERS_QUEUED, &Queued );
	return static_cast< BcU32 >( Queued );
}

//////////////////////////////////////////////////////////////////////////
// samplesComplete
BcU32 SsChannelAL::samplesComplete()
{
	ALint Complete;
	alGetSourcei( ALSource_, AL_BUFFERS_PROCESSED, &Complete );
	return static_cast< BcU32 >( Complete );
}

//////////////////////////////////////////////////////////////////////////
// play
void SsChannelAL::play( SsSampleAL* Sample, SsChannelCallback* Callback )
{
	// TODO: Lock/fence!!
	BcAssert( State_ == ssCS_IDLE || State_ == ssCS_STOPPED );
	alSourcei( ALSource_, AL_BUFFER, Sample->getHandle< ALuint >() );
	alSourcei( ALSource_, AL_LOOPING, Sample->isLooping() );
	
#if SS_AL_EFX_SUPPORTED
	if( SsCore::pImpl< SsCoreImplAL >()->isEFXEnabled() )
	{
		// NEILO TODO.
		//alSource3i( ALSource_, AL_AUXILIARY_SEND_FILTER, SsCore::pImpl< SsCoreImplAL >()->getALReverbAuxSlot(), 0, AL_FILTER_NULL );
	}
#endif
	alBreakOnError();

	Sample_ = Sample;
	pCallback_ = Callback;
	State_ = ssCS_PREPARED;

}

//////////////////////////////////////////////////////////////////////////
// update
void SsChannelAL::update()
{
	if( ALSource_ != 0 )
	{
		// Capture AL source state.
		// Check state of source and change state depending.
		ALint ALState;
		alGetSourcei( ALSource_, AL_SOURCE_STATE, &ALState );
		alBreakOnError();

		// Handle our own internal state.
		switch( State_ )
		{
		case ssCS_IDLE:
			{
				// Do nothing.
				return;
			}
			break;

		case ssCS_PREPARED:
			{
				// Trigger play.
				updateParams();
				alSourcePlay( ALSource_ );
				alBreakOnError();

				// Perform callback.
				if( getCallback() != NULL )
				{
					getCallback()->onStarted( this );
				}

				State_ = ssCS_PLAYING;
			}
			break;

		case ssCS_PLAYING:
			{
				// Update channel.
				updateParams();				

				if( getCallback() != NULL )
				{
					getCallback()->onPlaying( this );
				}

				// Handle AL source state changes.
				switch( ALState )
				{
				case AL_PLAYING:
					{
						State_ = ssCS_PLAYING;
					}
					break;

				case AL_PAUSED:
					{
						State_ = ssCS_PAUSED;
					}
					break;

				case AL_STOPPED:
					{
						State_ = ssCS_STOPPED;
					}
					break;
				}
			}
			break;

		case ssCS_PAUSED:
			{
				// Handle AL source state changes.
				switch( ALState )
				{
				case AL_PLAYING:
					{
						State_ = ssCS_PLAYING;
					}
					break;

				case AL_PAUSED:
					{
						State_ = ssCS_PAUSED;
					}
					break;

				case AL_STOPPED:
					{
						State_ = ssCS_STOPPED;
					}
					break;
				}
			}
			break;

		case ssCS_STOPPED:
			{
				// Free sound
				if( getCallback() != NULL )
				{
					getCallback()->onEnded( this );
				}

				// Free channel.
				Parent_->freeChannel( this );

				// Reset state to idle.
				State_ = ssCS_IDLE;
			}
			break;
		}

		/* Do we really need this?

		switch( ALState )
		{
		case AL_INITIAL:
			{
				
			}
			break;

		case AL_PLAYING:
			{
				updateParams();				
				
				if( getCallback() != NULL )
				{
					getCallback()->onPlaying( this );
				}
			}
			break;

		case AL_PAUSED:
			{
				// TODO: Nothing

			}
			break;

		case AL_STOPPED:
			{
				/*
				// Free sound
				if( getCallback() != NULL )
				{
					getCallback()->onEnded( this );
				}
				
				//
				Parent_->freeChannel( this );
			}
			break;
		}
		 */
	}
}

//////////////////////////////////////////////////////////////////////////
// updateParams
void SsChannelAL::updateParams()
{
	// Setup source parameters.
	alSourcef( ALSource_, AL_GAIN, Gain_ );
	alBreakOnError();
	alSourcef( ALSource_, AL_PITCH, Pitch_ );
	alBreakOnError();
	alSource3f( ALSource_, AL_POSITION, Position_.x(), Position_.y(), Position_.z() );
	alBreakOnError();

	alSourcef( ALSource_, AL_REFERENCE_DISTANCE, RefDistance_ );
	alBreakOnError();
	alSourcef( ALSource_, AL_MAX_DISTANCE, MaxDistance_ );
	alBreakOnError();
	alSourcef( ALSource_, AL_ROLLOFF_FACTOR, RolloffFactor_ );
	alBreakOnError();

#if SS_AL_EFX_SUPPORTED
	if( SsCore::pImpl< SsCoreImplAL >()->isEFXEnabled() )
	{
		// Final filter parameters.
		/*
		alFilteri( ALFilter_, AL_FILTER_TYPE, AL_FILTER_LOWPASS );
		alFilterf( ALFilter_, AL_LOWPASS_GAIN, 0.5f );
		alFilterf( ALFilter_, AL_LOWPASS_GAINHF, 1.0f );
		alSourcei( ALSource_, AL_DIRECT_FILTER, ALFilter_ );
		*/
	}
#endif
}
