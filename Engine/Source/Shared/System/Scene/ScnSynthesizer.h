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

#include "portaudio.h"

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
		CMD_NOTE_ON,				// Turn a note on. 2 param. MIDI NOTE.
		CMD_NOTE_OFF,				// Turn a note off.
		CMD_SET_ADSR,				// Set ADSR. 4 param.
		CMD_SET_GAIN,				// Set gain. 1 param.
		CMD_SET_LOW_PASS,			// Set low pass. 2 param.
		CMD_SET_HIGH_PASS,			// Set high pass. 2 param.
		CMD_SET_TEMPO,				// Set tempo. 1 param.
		CMD_SET_SYNTH_MODULE,		// Set synth module. 1 param.
		CMD_RESET					// Reset to start of track.
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
private:
	void								fileReady();
	void								fileChunkReady( const CsFileChunk* pChunk, void* pData );
	
	static int							streamCallback( const void *input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData );
	void								process( BcU32 Frames, BcReal* pFrames );
	
	void								processCommand();
	void								processAudio( BcU32 Frames, BcReal* pTempBuffer, BcReal* pOutputBuffer );
		
private:
	void								setTempo( BcReal Tempo );
	
private:
	typedef std::vector< class ScnSynthModule* > TSynthModuleList;
	typedef TSynthModuleList::iterator TSynthModuleListIterator;
	

	enum TConstants
	{
		MAX_COMMANDS = 64,
		MAX_TRACKS = 8,
		COMMANDS_PER_BEAT = 16,
	};
	
	struct TCommand
	{
		TCommandType					Type_;
		BcReal							Values_[ 4 ];
	};
	
	struct TTrack
	{
		class ScnSynthModule*			pModule_;
		TCommand						Commands_[ MAX_COMMANDS ];
	};
	
	BcReal								Tempo_;
	BcReal								SampleRate_;
	BcReal								SamplesPerBeat_;
	BcReal								SamplesPerCommand_;
	
	BcU32								CommandIndex_;
	BcReal								SamplesToProcess_;
	
	TSynthModuleList					SynthModules_;
	
	TTrack								CurrTracks_[ MAX_TRACKS ];
	TTrack								NextTracks_[ MAX_TRACKS ];
	
	BcAtomicBool						QueueNextPattern_;
	
	BcMutex								TrackLock_;
	
	
	PaStream*							pPaStream_;
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
	BcReal sampleCursor( BcBool Wrap = BcTrue ); // Returns between 0 and 2PI for where on a wave to sample.
	
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
		Env = BcLerp( 1.0f, 0.5f, ( Cursor_ - AEnd_ ) / D_ );
	}
	else if( Cursor_ < SEnd_ )
	{
		Env = 0.5f;
	}
	else if( Cursor_ < REnd_ )
	{
		Env = BcLerp( 0.5f, 0.0f, ( Cursor_ - SEnd_ ) / R_ );
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
