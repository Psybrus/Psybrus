/**************************************************************************
*
* File:		GaPlayerSoundComponent.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		Player component.
*		
*
*
* 
**************************************************************************/

#ifndef __GaPlayerSoundComponent_H__
#define __GaPlayerSoundComponent_H__

#include "Psybrus.h"

#include "GaPawnComponent.h"
#include "GaPlayerComponent.h"
#include "GaWorldBSPComponent.h"
#include "GaWorldPressureComponent.h"

#include <portaudio.h>

#include <aptk/filter.hh>
#include <aptk/lowpassres.hh>
#include <aptk/peakeq.hh>


//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef CsResourceRef< class GaPlayerSoundComponent > GaPlayerSoundComponentRef;


//////////////////////////////////////////////////////////////////////////
// GaPlayerSoundComponent
class GaPlayerSoundComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaPlayerSoundComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcF32 Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	BcVec3d								intersection( const BcVec3d& Direction ); 
	
	static int							streamCallback( const void *input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData );
	void								process( BcU32 Frames, BcF32* pFrames );

	eEvtReturn							onReset( EvtID ID, const GaWorldResetEvent& Event );

private:
	ScnCanvasComponentRef				Canvas_;
	GaWorldBSPComponentRef				BSP_;
	GaWorldPressureComponentRef			Pressure_;
	GaPlayerComponentRef				Player_;
	GaPawnComponentRef					Pawn_;

	BcU32								DoneReset_;
	BcU32								BufferSize_;
	BcU32								SampleRate_;
	PaStream*							pPaStream_;
	BcF32*								pWorkingBuffers_[ 2 ];

	BcRandom							NoiseGenerator_;
	
	BcAtomicMutex						FilterLock_;

	// Game thread.
	aptk::LowpassRes< BcF32, BcF32 >	LowPassPrevL_;
	aptk::LowpassRes< BcF32, BcF32 >	LowPassPrevR_;
	aptk::LowpassRes< BcF32, BcF32 >	LowPassCurrL_;
	aptk::LowpassRes< BcF32, BcF32 >	LowPassCurrR_;

	// Sound thread.
	aptk::LowpassRes< BcF32, BcF32 >	LowPassL_;
	aptk::LowpassRes< BcF32, BcF32 >	LowPassR_;
	aptk::PeakEQ< BcF32, BcF32 >		PeakL_;
	aptk::PeakEQ< BcF32, BcF32 >		PeakR_;
};

#endif

