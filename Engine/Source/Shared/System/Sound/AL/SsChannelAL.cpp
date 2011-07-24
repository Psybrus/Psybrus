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

#include "SsChannelAL.h"

#include "SsCoreALInternal.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
SsChannelAL::SsChannelAL( SsCoreALInternal* Parent ):
	ALSource_( NULL ),
	ALFilter_( NULL ),
	State_( ssCS_IDLE ),
	Sample_( NULL ),
	Parent_( Parent )
{
	// Generate source.
	alGenSources( 1, &ALSource_ );
	alBreakOnError();

#ifdef EX_AL_EFX
	// Generate filter (if supported)
	alGenFilters( 1, &ALFilter_ );
	alBreakOnError();
#endif
}

//////////////////////////////////////////////////////////////////////////
// Dtor
SsChannelAL::~SsChannelAL()
{
	// Delete source.
	alDeleteSources( 1, &ALSource_ );
	alBreakOnError();

#ifdef EX_AL_EFX
	// Delete filters.
	alDeleteFilters( 1, &ALFilter_ );
	alBreakOnError();
#endif
}

//////////////////////////////////////////////////////////////////////////
// stop
//virtual
void SsChannelAL::stop()
{
	alSourceStop( ALSource_ );
}

//////////////////////////////////////////////////////////////////////////
// play
void SsChannelAL::play( SsSampleAL* Sample, SsChannelCallback* Callback )
{
	BcAssert( State_ == ssCS_IDLE || State_ == ssCS_STOPPED );

	Sample_ = Sample;
	pCallback_ = Callback;
	State_ = ssCS_PREPARED;
	
	alSourcei( ALSource_, AL_BUFFER, Sample->getHandle< ALuint >() );
#ifdef EX_AL_EFX
	//alSource3i( ALSource_, AL_AUXILIARY_SEND_FILTER, ALReverbEffectSlot_, 0, EX_NULL );
#endif
	alBreakOnError();
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
	alSource3f( ALSource_, AL_POSITION, Position_.x(), Position_.y(), Position_.z() );
	alSourcei( ALSource_, AL_LOOPING, 0 );

#ifdef EX_AL_EFX
	// Final filter parameters.
	/*
	alFilteri( ALFilter_, AL_FILTER_TYPE, AL_FILTER_LOWPASS );
	alFilterf( ALFilter_, AL_LOWPASS_GAIN, LowpassGain_ );
	alFilterf( ALFilter_, AL_LOWPASS_GAINHF, LowpassGainHF_ );
	alSourcei( ALSource_, AL_DIRECT_FILTER, ALFilter_ );
	*/
#endif
}
