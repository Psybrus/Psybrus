/**************************************************************************
*
* File:		ScnSynthesizer.cpp
* Author:	Neil Richardson 
* Ver/Date:	17/08/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnSynthesizer.h"

#include "CsCore.h"

#ifdef PSY_SERVER
#include "BcFile.h"
#include "BcStream.h"
#include "json.h"
#endif


//////////////////////////////////////////////////////////////////////////
// PortAudio init/deinit.
static BcU32 PortAudioRefs = 0;
static void acquirePortAudio()
{
#ifndef PSY_SERVER
	if( PortAudioRefs == 0 )
	{
		Pa_Initialize();
	}	
#endif	
	++PortAudioRefs;
}

static void releasePortAudio()
{
	--PortAudioRefs;
	
#ifndef PSY_SERVER
	if( PortAudioRefs == 0 )
	{
		Pa_Terminate();
	}
#endif
}

#ifdef PSY_SERVER
//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnSynthesizer::import( const Json::Value& Object, CsDependancyList& DependancyList )
{
	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// MIDI table.
const BcReal ScnSynthesizer::MIDITable[ 128 ] =
{
	8.18f,
	8.66f,
	9.18f,
	9.72f,
	10.3f,
	10.91f,
	11.56f,
	12.25f,
	12.98f,
	13.75f,
	14.57f,
	15.43f,
	16.35f,
	17.32f,
	18.35f,
	19.45f,
	20.6f,
	21.83f,
	23.12f,
	24.5f,
	25.96f,
	27.5f,
	29.14f,
	30.87f,
	32.7f,
	34.65f,
	36.71f,
	38.89f,
	41.2f,
	43.65f,
	46.25f,
	49.0f,
	51.91f,
	55.0f,
	58.27f,
	61.74f,
	65.41f,
	69.3f,
	73.42f,
	77.78f,
	82.41f,
	87.31f,
	92.5f,
	98.0f,
	103.83f,
	110.0f,
	116.54f,
	123.47f,
	130.81f,
	138.59f,
	146.83f,
	155.56f,
	164.81f,
	174.61f,
	185.0f,
	196.0f,
	207.65f,
	220.0f,
	233.08f,
	246.94f,
	261.63f,
	277.18f,
	293.66f,
	311.13f,
	329.63f,
	349.23f,
	369.99f,
	392.0f,
	415.3f,
	440.0f,
	466.16f,
	493.88f,
	523.25f,
	554.37f,
	587.33f,
	622.25f,
	659.26f,
	698.46f,
	739.99f,
	783.99f,
	830.61f,
	880.0f,
	932.33f,
	987.77f,
	1046.5f,
	1108.73f,
	1174.66f,
	1244.51f,
	1318.51f,
	1396.91f,
	1479.98f,
	1567.98f,
	1661.22f,
	1760.0f,
	1864.66f,
	1975.53f,
	2093.0f,
	2217.46f,
	2349.32f,
	2489.02f,
	2637.02f,
	2793.83f,
	2959.96f,
	3135.96f,
	3322.44f,
	3520.0f,
	3729.31f,
	3951.07f,
	4186.01f,
	4434.92f,
	4698.64f,
	4978.03f,
	5274.04f,
	5587.65f,
	5919.91f,
	6271.93f,
	6644.88f,
	7040.0f,
	7458.62f,
	7902.13f,
	8372.02f,
	8869.84f,
	9397.27f,
	9956.06f,
	10548.08f,
	11175.3f,
	11839.82f,
	12543.85f
};

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnSynthesizer );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnSynthesizer::initialise( BcReal SampleRate )
{
	SampleRate_ = SampleRate;
	IsReady_ = BcFalse;	
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnSynthesizer::create()
{
	// Acquire portaudio.
	acquirePortAudio();
	
#ifndef PSY_SERVER
	// enum devices.
	PaDeviceIndex NoofDevice = Pa_GetDeviceCount();
	PaDeviceIndex DefaultOutputDevice = Pa_GetDefaultOutputDevice();
	
	
		
	// Create stream.
	PaError Error = Pa_OpenDefaultStream( &pPaStream_,
								          0,
								          1,
										  paFloat32,
								          SampleRate_,
								          2048,
								          &ScnSynthesizer::streamCallback,
								          this );
	BcAssert( Error == paNoError );
	
	Pa_StartStream( pPaStream_ );
#endif
	
	// Reset stuff.
	CommandIndex_ = 0;
	SamplesToProcess_ = 0.0f;
	//BcMemZero( &CurrPattern_, sizeof( CurrPattern_ ) );
	//BcMemZero( &NextPattern_, sizeof( NextPattern_ ) );

	// Default to 120BPM.
	setTempo( 120.0f );
	
	// Setup current pattern to reset so we queue up the tracks quick enough.
	CurrPattern_.Tracks_[ 0 ].Commands_[ 0 ].Type_ = CMD_RESET;
	CurrPattern_.Tracks_[ 0 ].Commands_[ 0 ].Values_[ 0 ] = 0.0f;
	
	// Delay buffer for sampling from later.
	pDelayBuffer_ = new DelayBuffer( BcU32( SampleRate_ ) * 8 );
	
	//
	IsReady_ = BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnSynthesizer::destroy()
{
#ifndef PSY_SERVER
	Pa_StopStream( pPaStream_ );
	Pa_CloseStream( pPaStream_ );
	pPaStream_ = NULL;
#endif	
	// Release portaudio.
	releasePortAudio();
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnSynthesizer::isReady()
{
	return IsReady_;
}

//////////////////////////////////////////////////////////////////////////
// setCommand
void ScnSynthesizer::setCommand( BcU32 TrackIdx, BcU32 CommandIdx, TCommandType Type, BcReal ValueA, BcReal ValueB, BcReal ValueC, BcReal ValueD )
{
	TrackLock_.lock();
	TTrack& Track = NextPattern_.Tracks_[ TrackIdx ];
	
	TCommand& Command = Track.Commands_[ CommandIdx ];
	
	Command.Type_ = Type;
	Command.Values_[ 0 ] = ValueA;
	Command.Values_[ 1 ] = ValueB;
	Command.Values_[ 2 ] = ValueC;
	Command.Values_[ 3 ] = ValueD;
	
	TrackLock_.unlock();
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnSynthesizer::nextPattern()
{
	TrackLock_.lock();
	PatternQueue_.push_back( NextPattern_ );
	TrackLock_.unlock();
}

//////////////////////////////////////////////////////////////////////////
// addSynthModule
BcU32 ScnSynthesizer::addSynthModule( TSynthModuleType Type )
{
	ScnSynthModule* pModule = NULL;
	
	switch( Type )
	{
		case MOD_SINE:
			pModule = new ScnSynthModuleSine( SampleRate_ );
			break;
		case MOD_SINE_RECT:
			pModule = new ScnSynthModuleSineRectified( SampleRate_ );
			break;
		case MOD_PULSE:
			pModule = new ScnSynthModulePulse( SampleRate_ );
			break;
		case MOD_SQUARE:
			pModule = new ScnSynthModuleSquare( SampleRate_ );
			break;
		case MOD_SAWTOOTH:
			pModule = new ScnSynthModuleSawtooth( SampleRate_ );
			break;
		case MOD_NOISE:
			pModule = new ScnSynthModuleNoise( SampleRate_ );
			break;
		default:
			return BcErrorCode;
			break;			
	}
	
	//
	SynthModules_.push_back( pModule );
	return SynthModules_.size() - 1;
}

//////////////////////////////////////////////////////////////////////////
// getFreq
BcReal ScnSynthesizer::getFreq( BcU32 MIDINote )
{
	if( MIDINote < 128 )
	{
		return MIDITable[ MIDINote ];
	}
	else
	{
		return 0.0f;
	}
}

//////////////////////////////////////////////////////////////////////////
// hadBeatEvent
BcBool ScnSynthesizer::hadBeatEvent()
{
	if( BeatsPassed_ != 0 )
	{
		--BeatsPassed_;
		return BcTrue;
	}
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// hadQueuePatternEvent
BcBool ScnSynthesizer::hadQueuePatternEvent()
{
	if( QueuePatternEvent_ == BcTrue )
	{
		QueuePatternEvent_ = BcFalse;
		return BcTrue;
	}
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// hadLastPatternEvent
BcBool ScnSynthesizer::hadLastPatternEvent()
{
	if( LastPatternEvent_ == BcTrue )
	{
		LastPatternEvent_ = BcFalse;
		return BcTrue;
	}
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// getCommandIndex
BcU32 ScnSynthesizer::getCommandIndex() const
{
	return CommandIndex_;
}

//////////////////////////////////////////////////////////////////////////
// setReverb
void ScnSynthesizer::setReverb( BcReal Delay, BcReal Feedback, BcReal Wet, BcReal Dry )
{
	ReverbDelay_ = Delay;
	ReverbFeedback_ = Feedback;
	ReverbWet_ = Wet;
	ReverbDry_ = Dry;
}

//////////////////////////////////////////////////////////////////////////
// playEffect
void ScnSynthesizer::playEffect( BcU32 Track, BcReal StartFreq, BcReal EndFreq )
{
	TrackLock_.lock();
	ScnSynthModule* pModule = CurrPattern_.Tracks_[ Track ].pModule_;
	if( pModule != NULL )
	{
		pModule->setFrequency( StartFreq, EndFreq );
		pModule->keyOn();
	}
	TrackLock_.unlock();
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnSynthesizer::fileReady()
{
	// File is ready, get the header chunk.
	CsResource::getChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnSynthesizer::fileChunkReady( const CsFileChunk* pChunk, void* pData )
{
	if( pChunk->ID_ == BcHash( "header" ) )
	{
		
	}
}

//////////////////////////////////////////////////////////////////////////
// streamCallback
#ifndef PSY_SERVER
int ScnSynthesizer::streamCallback( const void *input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData )
{
	ScnSynthesizer* pSynthesizer = (ScnSynthesizer*)userData;
	BcReal* pOutput = (BcReal*)output;
	pSynthesizer->process( frameCount, pOutput );
	return paContinue;
}
#endif
//////////////////////////////////////////////////////////////////////////
// process
void ScnSynthesizer::process( BcU32 NoofFrames, BcReal* pFrames )
{
	BcU32 Frames = NoofFrames;
	// Clear output buffer.
	BcMemZero( pFrames, Frames * sizeof( BcReal ) );

	// Temporary output buffer for rendering.
	BcReal* pTempBuffer = new BcReal[ Frames ];
	BcReal* pOutputCursor = pFrames;
	
	// Process the slack from leftover from previous process.
	if( SamplesToProcess_ > 0.0f )
	{
		processAudio( BcU32( SamplesToProcess_ ), pTempBuffer, pOutputCursor );
		
		pOutputCursor += BcU32( SamplesToProcess_ );
		Frames -= BcU32( SamplesToProcess_ );
		SamplesToProcess_ -= BcU32( SamplesToProcess_ );
	}
		

	// Process in batches the size of commands.
	while( Frames > 0 )
	{
		// Cache samples per command as the command can change it. (should handle it really)
		BcReal SamplesPerCommand = BcMin( (BcReal)Frames, SamplesPerCommand_ );
				
		// Push samples to process up by samples per command.
		SamplesToProcess_ += SamplesPerCommand_;

		// Process the command.
		processCommand();

		// Process the audio.
		processAudio( BcU32( SamplesPerCommand ), pTempBuffer, pOutputCursor );
		
		// Decrement samples to process, and increment cursor.
		pOutputCursor += BcU32( SamplesPerCommand );
		SamplesToProcess_ -= BcU32( SamplesPerCommand );
		Frames -= BcU32( SamplesPerCommand );
	}
	
#if PSY_DEBUG
	if( SamplesToProcess_ > SamplesPerCommand_ )
	{
		BcPrintf( "BADNESS: SamplesToProcess_ > SamplesPerCommand_ at end of processing.\n" );
	}
#endif

	
	// Free output buffer.
	delete pTempBuffer;
}

//////////////////////////////////////////////////////////////////////////
// processCommand
void ScnSynthesizer::processCommand()
{
	BcU32 CommandIndex = CommandIndex_++;
	
	if( ( CommandIndex % COMMANDS_PER_BEAT ) == 0 )
	{
		++BeatsPassed_;
	}
		
	for( BcU32 TrackIdx = 0; TrackIdx < MAX_TRACKS; ++TrackIdx )
	{
		TTrack& Track = CurrPattern_.Tracks_[ TrackIdx ];
		TTrack& NextTrack = NextPattern_.Tracks_[ TrackIdx ];
		
		TCommand& Command = Track.Commands_[ CommandIndex ];
		ScnSynthModule* pModule = Track.pModule_;
		
		switch( Command.Type_ )
		{
			case CMD_NONE:
				break;
			case CMD_NOTE_ON:
				if( pModule != NULL )
				{
					pModule->setFrequency( Command.Values_[ 0 ], Command.Values_[ 1 ] );
					pModule->keyOn();
				}
				break;
			case CMD_NOTE_OFF:
				if( pModule != NULL )
				{
					pModule->keyOff();
				}
				break;
			case CMD_NOTE_SET:
				if( pModule != NULL )
				{
					pModule->setFrequency( Command.Values_[ 0 ], Command.Values_[ 1 ] );
				}
				break;
			case CMD_SET_ADSR:
				if( pModule != NULL )
				{
					pModule->setADSR( Command.Values_[ 0 ], Command.Values_[ 1 ], Command.Values_[ 2 ], Command.Values_[ 3 ] );
				}
				break;
			case CMD_SET_GAIN:
				if( pModule != NULL )
				{
					pModule->setGain( Command.Values_[ 0 ] );
				}
				break;
			case CMD_SET_LOW_PASS:
				if( pModule != NULL )
				{
					if( Track.LFOEnabled_ == BcFalse )
					{
						pModule->setLowPass( Command.Values_[ 0 ], Command.Values_[ 1 ] );
					}
					Track.FilterType_ = Command.Type_;
					Track.Coeff_ = Command.Values_[ 0 ];
					Track.Res_ = Command.Values_[ 1 ];
				}
				break;
			case CMD_SET_HIGH_PASS:
				if( pModule != NULL )
				{
					if( Track.LFOEnabled_ == BcFalse )
					{
						pModule->setHighPass( Command.Values_[ 0 ], Command.Values_[ 1 ] );
					}
					Track.FilterType_ = Command.Type_;
					Track.Coeff_ = Command.Values_[ 0 ];
					Track.Res_ = Command.Values_[ 1 ];
				}
				break;
			case CMD_SET_TEMPO:
				setTempo( Command.Values_[ 0 ] );
				break;
			case CMD_SET_SYNTH_MODULE:
				{
					BcU32 SynthModuleIdx = BcU32( BcRound( Command.Values_[ 0 ] ) );
					if( SynthModuleIdx < SynthModules_.size() )
					{
						Track.pModule_ = SynthModules_[ SynthModuleIdx ];
						NextTrack.pModule_ = Track.pModule_;
					}
				}
				break;
			case CMD_SET_LFO:
				{
					Track.LFOSpeed_ = Command.Values_[ 0 ];
					Track.LFOMultiplier_ = Command.Values_[ 1 ];
					if( Command.Values_[ 2 ] > 0.0f )
					{
						Track.LFOCursor_ = Command.Values_[ 2 ];
					}
				}
				break;

			case CMD_RESET:
				CommandIndex_ = MAX_COMMANDS + Command.Values_[ 0 ]; 
				break;
		}
		
		// Do LFO.
		if( Track.LFOEnabled_ )
		{
			BcReal LFOCycle = ( BcPIMUL2 * Track.LFOSpeed_ ) / (BcReal)MAX_COMMANDS;
			Track.LFOCursor_ += LFOCycle;
			BcReal LFOAmplitude = ( BcSin( Track.LFOCursor_ ) * Track.LFOMultiplier_ ) + 1.0f;
			BcReal Coeff = Track.Coeff_ * LFOAmplitude;
			if( pModule != NULL )
			{
				if( Track.FilterType_ == CMD_SET_LOW_PASS )
				{ 
					pModule->setLowPass( Coeff, Track.Res_ );
				}
				else if( Track.FilterType_ == CMD_SET_HIGH_PASS )
				{
					pModule->setHighPass( Coeff, Track.Res_ );
				}
			}
		}
	}
	
	// Handle resetting.
	if( CommandIndex_ >= MAX_COMMANDS )
	{
		CommandIndex_ -= MAX_COMMANDS;
		
		// If we need to queue next track, do so!
		TrackLock_.lock();
		if( PatternQueue_.size() > 0 )
		{
			TPattern& NextPattern = PatternQueue_[ 0 ];
			for( BcU32 TrackIdx = 0; TrackIdx < MAX_TRACKS; ++TrackIdx )
			{
				for( BcU32 CommandIdx = 0; CommandIdx < MAX_COMMANDS; ++CommandIdx )
				{
					CurrPattern_.Tracks_[ TrackIdx ].Commands_[ CommandIdx ] = NextPattern.Tracks_[ TrackIdx ].Commands_[ CommandIdx ];
				}
			}
			PatternQueue_.pop_front();
			
			QueuePatternEvent_ = BcTrue;
			if( PatternQueue_.size() == 0 )
			{
				LastPatternEvent_ = BcTrue;
			}
		}
		TrackLock_.unlock();
	}
}

//////////////////////////////////////////////////////////////////////////
// processAudio
void ScnSynthesizer::processAudio( BcU32 Frames, BcReal* pTempBuffer, BcReal* pOutputBuffer )
{
	for( BcU32 TrackIdx = 0; TrackIdx < MAX_TRACKS; ++TrackIdx )
	{
		TTrack& Track = CurrPattern_.Tracks_[ TrackIdx ];
		
		BcMemZero( pTempBuffer, sizeof( BcReal ) * Frames );
		
		// Process into temp buffer.
		if( Track.pModule_ != NULL )
		{
			Track.pModule_->process( Frames, pTempBuffer );
		}
		
		// Mix with output buffer.
		// TODO: Optimise.
		for( BcU32 FrameIdx = 0; FrameIdx < Frames; ++FrameIdx )
		{
			pOutputBuffer[ FrameIdx ] += pTempBuffer[ FrameIdx ];
		}		
	}

	// Post process.
	setReverb( SampleRate_ / 8.0f, 0.6f, 0.2f, 0.8f );
	BcRandom CombNoise;
	BcReal Delay = ReverbDelay_;
	BcReal Gain = 1.0f;
	BcReal TotalReverbGain = 0.0f;
	for( BcU32 Idx = 0; Idx < REVERB_TAPS; ++Idx )
	{
		ReverbTapDelays_[ Idx ] = Delay + ( CombNoise.interpolatedNoise( Delay, SampleRate_ ) * ReverbDelay_ );
		ReverbTapGains_[ Idx ] = ( Idx & 1 ) ? Gain * 0.5f : Gain;
		Delay += ReverbDelay_;
		TotalReverbGain += Gain;
		Gain *= 0.8f;
	}
	TotalReverbGain = 1.0f / TotalReverbGain;
	
	for( BcU32 FrameIdx = 0; FrameIdx < Frames; ++FrameIdx )
	{
		register BcReal Output = pOutputBuffer[ FrameIdx ];

		// Push into delay buffer with a delay amount mixed in + poor mans lowpass.
		pDelayBuffer_->push( Output + ( ( pDelayBuffer_->get( ReverbDelay_ ) + pDelayBuffer_->get( ReverbDelay_ + 1.0f ) + pDelayBuffer_->get( ReverbDelay_ + 2.0f ) + pDelayBuffer_->get( ReverbDelay_ + 3.0f ) ) * 0.25f * ReverbFeedback_ ) );

		// Sample reverb.
		register BcReal ReverbOut = 0.0f;
		for( BcU32 ReverbTapIdx = 0; ReverbTapIdx < REVERB_TAPS; ++ReverbTapIdx )
		{
			ReverbOut += pDelayBuffer_->get( ReverbTapDelays_[ ReverbTapIdx ] ) * ReverbTapGains_[ ReverbTapIdx ];
		}
		
		Output = ( ReverbDry_ * Output ) + ( ReverbWet_ * ReverbOut * TotalReverbGain );
		
		pOutputBuffer[ FrameIdx ] = Output;
	}		
}

//////////////////////////////////////////////////////////////////////////
// setTempo
void ScnSynthesizer::setTempo( BcReal Tempo )
{
	Tempo_ = Tempo;
	SamplesPerBeat_ = SampleRate_ / ( Tempo_ / 60.0f );
	SamplesPerCommand_ = SamplesPerBeat_ / (BcReal)COMMANDS_PER_BEAT;
}

//////////////////////////////////////////////////////////////////////////
// ScnSynthModule
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnSynthModule::ScnSynthModule( BcReal SampleRate ):
	Active_( BcFalse ),
	Cursor_( 0.0f ),
	Gain_( 0.0f ),
	StartCycle_( 0.0f ),
	EndCycle_( 0.0f ),
	Cycle_( 0.0f ),
	Timer_( 0.0f ),
	A_( 0.0f ),
	D_( 0.0f ),
	S_( 0.0f ),
	R_( 0.0f ),
	AEnd_( 0.0f ),
	DEnd_( 0.0f ),
	SEnd_( 0.0f ),
	REnd_( 0.0f ),
	TotalTime_( 0.0f ),
	SampleRate_( SampleRate )
{
	BcMemSet( FilterInCoef_, 0, sizeof( FilterInCoef_ ) );
	BcMemSet( FilterOutCoef_, 0, sizeof( FilterOutCoef_ ) );
	BcMemSet( FilterInBuf_, 0, sizeof( FilterInBuf_ ) );
	BcMemSet( FilterOutBuf_, 0, sizeof( FilterOutBuf_ ) );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnSynthModule::~ScnSynthModule()
{
	
}

//////////////////////////////////////////////////////////////////////////
// keyOn
void ScnSynthModule::keyOn()
{
	//
	Active_ = BcTrue;
	Cursor_ = 0.0f;
	Timer_ = 0.0f;

	// Reset filter buffers.
	BcMemSet( FilterInBuf_, 0, sizeof( FilterInBuf_ ) );
	BcMemSet( FilterOutBuf_, 0, sizeof( FilterOutBuf_ ) );
}

//////////////////////////////////////////////////////////////////////////
// keyOff
void ScnSynthModule::keyOff()
{
	Cursor_ = R_;
}

//////////////////////////////////////////////////////////////////////////
// setFrequency
void ScnSynthModule::setFrequency( BcReal Start, BcReal End )
{
	StartCycle_ = BcPIMUL2 / ( SampleRate_ / Start );
	EndCycle_ = BcPIMUL2 / ( SampleRate_ / End );
}

//////////////////////////////////////////////////////////////////////////
// setGain
void ScnSynthModule::setGain( BcReal Gain )
{
	Gain_ = Gain;
}

//////////////////////////////////////////////////////////////////////////
// setADSR
void ScnSynthModule::setADSR( BcReal A, BcReal D, BcReal S, BcReal R )
{
	A_ = ( A * 0.001f ) * SampleRate_;
	D_ = ( D * 0.001f ) * SampleRate_;
	S_ = ( S * 0.001f ) * SampleRate_;
	R_ = ( R * 0.001f ) * SampleRate_;
	AEnd_ = A_;
	DEnd_ = A_ + D_;
	SEnd_ = A_ + D_ + S_;
	REnd_ = A_ + D_ + S_ + R_;
	TotalTime_ = A_ + D_ + S_ + R_;
}

//////////////////////////////////////////////////////////////////////////
// setLowPass
void ScnSynthModule::setLowPass( BcReal Coeff, BcReal Res )
{
	const BcReal lInitCoef = (1.0f / (BcTan(BcPI * BcClamp( Coeff, 0.01f, SampleRate_ * 0.49999f ) / SampleRate_)));
	const BcReal lInitCoefPw2 = lInitCoef * lInitCoef;
	const BcReal lResonance = (((BcSqrt(2.0f) - 0.1f) * (1.0f - Res)) + 0.1f);
	
	FilterInCoef_[0] = (1.0f / (1.0f + (lResonance * lInitCoef) + lInitCoefPw2));
	FilterInCoef_[1] = 2.0f * FilterInCoef_[0];
	FilterInCoef_[2] = FilterInCoef_[0];
	
	FilterOutCoef_[0] = (2.0f * FilterInCoef_[0] * (1.0f - lInitCoefPw2));
	FilterOutCoef_[1] = (FilterInCoef_[0] * (1.0f - lResonance * lInitCoef + lInitCoefPw2));
}

//////////////////////////////////////////////////////////////////////////
// setHighPass
void ScnSynthModule::setHighPass( BcReal Coeff, BcReal Res )
{
	const BcReal lInitCoef = ((BcTan(BcPI * BcClamp( Coeff, 0.01f, SampleRate_ * 0.49999f ) / SampleRate_)));
	const BcReal lInitCoefPw2 = lInitCoef * lInitCoef;
	const BcReal lResonance = (((BcSqrt(2.0f) - 0.1f) * (1.0f - Res)) + 0.1f);
	
	FilterInCoef_[0] = (1.0f / (1.0f + (lResonance * lInitCoef) + lInitCoefPw2));
	FilterInCoef_[1] = -2.0f * FilterInCoef_[0];
	FilterInCoef_[2] = FilterInCoef_[0];
	
	FilterOutCoef_[0] = (2.0f * FilterInCoef_[0] * (lInitCoefPw2 - 1.0f));
	FilterOutCoef_[1] = (FilterInCoef_[0] * (1.0f - lResonance * lInitCoef + lInitCoefPw2));
}

//////////////////////////////////////////////////////////////////////////
// process
void ScnSynthModule::process( BcU32 Frames, BcReal* pOutput )
{
	// Skip processing if we have nothing.
	if( Active_ == BcTrue )
	{
		// Do internal process.
		internalProcess( Frames, pOutput );
	
		// Clamp to range and filter..
		for( BcU32 Idx = 0; Idx < Frames; ++Idx )
		{
			register BcReal Sample = pOutput[ Idx ];
	
			// Calculate the new sample
			BcReal FilteredSample = ( FilterInCoef_[0] * Sample +
									  FilterInCoef_[1] * FilterInBuf_[0] +
									  FilterInCoef_[2] * FilterInBuf_[1] -
									  FilterOutCoef_[0] * FilterOutBuf_[0] -
									  FilterOutCoef_[1] * FilterOutBuf_[1] );
		
			// Shift along the input buffer
			FilterInBuf_[1] = FilterInBuf_[0];
			FilterInBuf_[0] = Sample;
		
			// Shift along the output buffer
			FilterOutBuf_[1] = FilterOutBuf_[0];
			FilterOutBuf_[0] = FilteredSample;
		
			pOutput[ Idx ] = BcClamp( FilteredSample * Gain_, -1.0, 1.0 );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// ScnSynthModuleSine
void ScnSynthModuleSine::internalProcess( BcU32 Frames, BcReal* pOutput )
{
	for( BcU32 Idx = 0; Idx < Frames; ++Idx )
	{
		register BcReal ADSR = sampleADSR();
		register BcReal Sample = BcSin( sampleCursor() );
		*pOutput++ = Sample * ADSR;
	}
}

//////////////////////////////////////////////////////////////////////////
// ScnSynthModuleSineRectified
void ScnSynthModuleSineRectified::internalProcess( BcU32 Frames, BcReal* pOutput )
{
	for( BcU32 Idx = 0; Idx < Frames; ++Idx )
	{
		register BcReal ADSR = sampleADSR();
		register BcReal Sample = BcAbs( BcSin( sampleCursor() ) );
		*pOutput++ = Sample * ADSR;
	}
}

//////////////////////////////////////////////////////////////////////////
// ScnSynthModulePulse
void ScnSynthModulePulse::internalProcess( BcU32 Frames, BcReal* pOutput )
{
	for( BcU32 Idx = 0; Idx < Frames; ++Idx )
	{
		register BcReal ADSR = sampleADSR();
		register BcReal Sample = BcSin( sampleCursor() );
		*pOutput++ = Sample * ADSR;
	}
}

//////////////////////////////////////////////////////////////////////////
// ScnSynthModuleSquare
void ScnSynthModuleSquare::internalProcess( BcU32 Frames, BcReal* pOutput )
{
	for( BcU32 Idx = 0; Idx < Frames; ++Idx )
	{
		register BcReal ADSR = sampleADSR();
		register BcReal Sample = ( sampleCursor( BcTrue ) ) < BcPI ? -1.0f : 1.0f;
		*pOutput++ = Sample * ADSR;
	}
}

//////////////////////////////////////////////////////////////////////////
// ScnSynthModuleSawtooth
void ScnSynthModuleSawtooth::internalProcess( BcU32 Frames, BcReal* pOutput )
{
	for( BcU32 Idx = 0; Idx < Frames; ++Idx )
	{
		register BcReal ADSR = sampleADSR();
		register BcReal Cursor = sampleCursor( BcTrue );
		register BcReal Sample = Cursor < BcPI ? ( Cursor / BcPI ) : ( ( Cursor - BcPI ) / BcPI ) - 1.0f;
		*pOutput++ = Sample * ADSR;
	}
}

//////////////////////////////////////////////////////////////////////////
// ScnSynthModuleNoise
void ScnSynthModuleNoise::internalProcess( BcU32 Frames, BcReal* pOutput )
{
	for( BcU32 Idx = 0; Idx < Frames; ++Idx )
	{
		register BcReal ADSR = sampleADSR();
		register BcReal Sample = Noise_.interpolatedNoise( sampleCursor( BcFalse ), (BcU32)SampleRate_ );
		*pOutput++ = Sample * ADSR;
	}
}
 