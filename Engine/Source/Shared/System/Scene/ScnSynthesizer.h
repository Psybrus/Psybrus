/**************************************************************************
*
* File:		ScnSynthesizer.h
* Author:	Neil Richardson 
* Ver/Date:	17/08/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnSynthesizer_H__
#define __ScnSynthesizer_H__

#include "CsResourceRef.h"

#include "BcMath.h"
#include "BcRandom.h"

#ifndef PSY_SERVER
#include "portaudio.h"
#endif

//////////////////////////////////////////////////////////////////////////
// ScnSynthesizerRef
typedef CsResourceRef< class ScnSynthesizer > ScnSynthesizerRef;

//////////////////////////////////////////////////////////////////////////
// ScnSynthesizer
class ScnSynthesizer:
	public CsResource
{
public:
	static const BcReal MIDITable[ 128 ];

	enum TCommandType
	{
		CMD_NONE = 0,				// No command.
		CMD_NOTE_ON,				// Turn a note on. 2 param.
		CMD_NOTE_OFF,				// Turn a note off.
		CMD_NOTE_SET,				// Set note without keying. 2 param.
		CMD_SET_ADSR,				// Set ADSR. 4 param.
		CMD_SET_GAIN,				// Set gain. 1 param.
		CMD_SET_LOW_PASS,			// Set low pass. 2 param.
		CMD_SET_HIGH_PASS,			// Set high pass. 2 param.
		CMD_SET_TEMPO,				// Set tempo. 1 param.
		CMD_SET_SYNTH_MODULE,		// Set synth module. 1 param.
		CMD_SET_LFO,				// Set LFO. 3 param.
		CMD_RESET					// Reset to start of track.
	};
	
	enum TSynthModuleType
	{
		MOD_SINE = 0,
		MOD_SINE_RECT,
		MOD_PULSE,
		MOD_SQUARE,
		MOD_SAWTOOTH,
		MOD_NOISE
	};

public:
	DECLARE_RESOURCE( CsResource, ScnSynthesizer );
	
#ifdef PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
#endif
	virtual void						initialise( BcReal SampleRate );
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
	void								setCommand( BcU32 TrackIdx, BcU32 CommandIdx, TCommandType Type, BcReal ValueA = 0.0f, BcReal ValueB = 0.0f, BcReal ValueC = 0.0f, BcReal ValueD = 0.0f);
	void								nextPattern();
	BcU32								addSynthModule( TSynthModuleType Type );
	BcReal								getFreq( BcU32 MIDINote );
	BcBool								hadBeatEvent();
	BcBool								hadQueuePatternEvent();
	BcBool								hadLastPatternEvent();
	BcU32								getCommandIndex() const;
	void								setReverb( BcReal Delay, BcReal Feedback, BcReal Wet, BcReal Dry );

	void								playEffect( BcU32 Track, BcReal StartFreq, BcReal EndFreq );
	
private:
	void								fileReady();
	void								fileChunkReady( const CsFileChunk* pChunk, void* pData );
	
#ifndef PSY_SERVER
	static int							streamCallback( const void *input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData );
#endif
	void								process( BcU32 Frames, BcReal* pFrames );
	
	void								processCommand();
	void								processAudio( BcU32 Frames, BcReal* pTempBuffer, BcReal* pOutputBuffer );
		
private:
	void								setTempo( BcReal Tempo );
	
private:
	typedef std::vector< class ScnSynthModule* > TSynthModuleList;
	typedef TSynthModuleList::iterator TSynthModuleListIterator;
	
	class DelayBuffer
	{
	public:
		BcForceInline DelayBuffer( BcU32 Size )
		{
			pBufStart_ = new BcReal[ Size ];
			pBufEnd_ = pBufStart_ + Size;
			pBufPos_ = pBufStart_;
			BcMemZero( pBufStart_, sizeof( BcReal ) * Size );
			Size_ = Size;
		}
		
		BcForceInline ~DelayBuffer()
		{
			delete pBufStart_;
		}
		
		BcForceInline void push( BcReal Sample )
		{
			*pBufPos_++ = Sample;
			if( pBufPos_ == pBufEnd_ )
			{
				pBufPos_ = pBufStart_;
			}
		}
		
		BcForceInline BcReal get( BcReal Time ) const
		{
			int TimeFloor = (int)Time;
			BcReal TimeLerp = Time - (BcReal)TimeFloor;
			BcReal* pSampleA = ( pBufPos_ - TimeFloor ) - 2;
			BcReal* pSampleB = ( pBufPos_ - TimeFloor ) - 1;
			while( pSampleA < pBufStart_ )
				pSampleA += Size_;
			while( pSampleB < pBufStart_ )
				pSampleB += Size_;
			return ( *pSampleA + ( ( *pSampleA - *pSampleB ) * TimeLerp ) );			
		}
		
	private:
		BcReal* pBufStart_;
		BcReal* pBufEnd_;
		BcReal* pBufPos_;
		BcU32 Size_;
	};
	
	
	enum TConstants
	{
		MAX_COMMANDS = 64,
		MAX_TRACKS = 16,
		COMMANDS_PER_BEAT = 16,
		REVERB_TAPS = 4
	};
	
	struct TCommand
	{
		TCommand():
			Type_( CMD_NONE )
		{}
		TCommandType					Type_;
		BcReal							Values_[ 4 ];
	};
	
	struct TTrack
	{
		TTrack():
			pModule_( NULL ),
			LFOEnabled_( BcTrue ),
			LFOSpeed_( 1.0f ),
			LFOCursor_( 0.0f ),
			LFOMultiplier_( 0.0f )
		{}
		class ScnSynthModule*			pModule_;
		TCommand						Commands_[ MAX_COMMANDS ];
		
		// Filter.
		TCommandType					FilterType_;
		BcReal							Coeff_;
		BcReal							Res_;

		// LFO.
		BcBool							LFOEnabled_;
		BcReal							LFOSpeed_;
		BcReal							LFOCursor_;
		BcReal							LFOMultiplier_;
	};
	
	BcReal								Tempo_;
	BcReal								SampleRate_;
	BcReal								SamplesPerBeat_;
	BcReal								SamplesPerCommand_;
	
	BcU32								CommandIndex_;
	BcReal								SamplesToProcess_;
	
	TSynthModuleList					SynthModules_;
	
	struct TPattern
	{
		TTrack							Tracks_[ MAX_TRACKS ];
	};
	
	TPattern							CurrPattern_;
	std::deque< TPattern >				PatternQueue_;
	
	// We write to this before queuing up.
	TPattern							NextPattern_;
	BcMutex								TrackLock_;
	
	// Beat queuing.
	BcAtomicU32							BeatsPassed_;
	BcAtomicBool						QueuePatternEvent_;
	BcAtomicBool						LastPatternEvent_;
	BcAtomicBool						IsReady_;
	
#ifndef PSY_SERVER
	PaStream*							pPaStream_;
#endif
	
	// Post processing.
	DelayBuffer*						pDelayBuffer_;
	
	BcReal								ReverbTapDelays_[ REVERB_TAPS ];
	BcReal								ReverbTapGains_[ REVERB_TAPS ];
	
	BcReal								ReverbDelay_;
	BcReal								ReverbFeedback_;
	BcReal								ReverbWet_;
	BcReal								ReverbDry_;
};

//////////////////////////////////////////////////////////////////////////
// ScnSynthModule
class ScnSynthModule
{
public:
	ScnSynthModule( BcReal SampleRate );
	virtual ~ScnSynthModule();
	
	void keyOn();
	void keyOff();
	
	void setFrequency( BcReal Start, BcReal End );
	void setGain( BcReal Gain );
	void setADSR( BcReal A, BcReal D, BcReal S, BcReal R );
	
	void setLowPass( BcReal Coeff, BcReal Res );
	void setHighPass( BcReal Coeff, BcReal Res );
	
	void process( BcU32 Frames, BcReal* pOutput );
	
	virtual void internalProcess( BcU32 Frames, BcReal* pOutput ) = 0;

protected:
	BcReal sampleADSR();
	BcReal sampleCursor( BcBool Wrap = BcFalse ); // Returns between 0 and 2PI for where on a wave to sample.
	
protected:
	BcBool Active_;
	BcReal Cursor_;
	BcReal Gain_;
	
	BcReal StartCycle_;
	BcReal EndCycle_;
	BcReal Cycle_;
	BcReal Timer_;
	
	BcReal A_, D_, S_, R_;
	BcReal AEnd_, DEnd_, SEnd_, REnd_;
	
	BcReal TotalTime_;
	BcReal SampleRate_;
	
	BcReal FilterInCoef_[3];
	BcReal FilterOutCoef_[3];
	BcReal FilterInBuf_[2];
	BcReal FilterOutBuf_[2];
};

//////////////////////////////////////////////////////////////////////////
// sampleADSR
BcForceInline BcReal ScnSynthModule::sampleADSR()
{
	BcReal Env = 0.0f;
	
	if( Cursor_ < AEnd_ )
	{
		Env = BcLerp( 0.0f, 1.0f, ( Cursor_ ) / A_ );
	}
	else if( Cursor_ < DEnd_ )
	{
		Env = BcLerp( 1.0f, 0.9f, ( Cursor_ - AEnd_ ) / D_ );
	}
	else if( Cursor_ < SEnd_ )
	{
		Env = 0.9f;
	}
	else if( Cursor_ < REnd_ )
	{
		Env = BcLerp( 0.9f, 0.0f, ( Cursor_ - SEnd_ ) / R_ );
	}
	else
	{
		Active_ = BcFalse;
	}
	
	return Env;
}

//////////////////////////////////////////////////////////////////////////
// sampleCursor
BcForceInline BcReal ScnSynthModule::sampleCursor( BcBool Wrap )
{
	// Cache return value.
	const BcReal RetVal = Timer_;
	
	// Calculate the cycle advance time depending on the frequency range specified.
	const BcReal LerpVal = ( Cursor_ / TotalTime_ );
	Cycle_ = BcLerp( StartCycle_, EndCycle_, LerpVal );
	
	// Advance timer by the defined cycle length.
	Timer_ += Cycle_;
	
	// Wrap round to prevent loss of precision if desired.
	if( Wrap == BcTrue && Timer_ > BcPIMUL2 )
	{
		Timer_ -= BcPIMUL2;
	}
	
	// Advance cursor.
	Cursor_ += 1.0f;

	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// ScnSynthModuleSine
class ScnSynthModuleSine: public ScnSynthModule
{
public:
	ScnSynthModuleSine( BcReal SampleRate ):
		ScnSynthModule( SampleRate )
	{
	}
	
private:
	void internalProcess( BcU32 Frames, BcReal* pOutput );
};

//////////////////////////////////////////////////////////////////////////
// ScnSynthModuleSineRectified
class ScnSynthModuleSineRectified: public ScnSynthModule
{
public:
	ScnSynthModuleSineRectified( BcReal SampleRate ):
		ScnSynthModule( SampleRate )
	{
	}
	
private:
	void internalProcess( BcU32 Frames, BcReal* pOutput );
};

//////////////////////////////////////////////////////////////////////////
// ScnSynthModulePulse
class ScnSynthModulePulse: public ScnSynthModule
{
public:
	ScnSynthModulePulse( BcReal SampleRate ):
		ScnSynthModule( SampleRate )
	{
	}
	
private:
	void internalProcess( BcU32 Frames, BcReal* pOutput );
};

//////////////////////////////////////////////////////////////////////////
// ScnSynthModuleSquare
class ScnSynthModuleSquare: public ScnSynthModule
{
public:
	ScnSynthModuleSquare( BcReal SampleRate ):
		ScnSynthModule( SampleRate )
	{
	}
	
private:
	void internalProcess( BcU32 Frames, BcReal* pOutput );
};

//////////////////////////////////////////////////////////////////////////
// ScnSynthModuleSawtooth
class ScnSynthModuleSawtooth: public ScnSynthModule
{
public:
	ScnSynthModuleSawtooth( BcReal SampleRate ):
		ScnSynthModule( SampleRate )
	{
	}
	
private:
	void internalProcess( BcU32 Frames, BcReal* pOutput );
};

//////////////////////////////////////////////////////////////////////////
// ScnSynthModuleNoise
class ScnSynthModuleNoise: public ScnSynthModule
{
public:
	ScnSynthModuleNoise( BcReal SampleRate ):
		ScnSynthModule( SampleRate )
	{
	}
	
private:
	void internalProcess( BcU32 Frames, BcReal* pOutput );
	
private:
	BcRandom Noise_;
};

#endif
