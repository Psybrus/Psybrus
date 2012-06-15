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

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	BcVec3d								intersection( const BcVec3d& Direction ); 
	
	static int							streamCallback( const void *input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData );
	void								process( BcU32 Frames, BcReal* pFrames );

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
	BcReal*								pWorkingBuffers_[ 2 ];

	BcRandom							NoiseGenerator_;
	
	BcAtomicMutex						FilterLock_;

	// Game thread.
	aptk::LowpassRes< BcReal, BcReal >	LowPassPrevL_;
	aptk::LowpassRes< BcReal, BcReal >	LowPassPrevR_;
	aptk::LowpassRes< BcReal, BcReal >	LowPassCurrL_;
	aptk::LowpassRes< BcReal, BcReal >	LowPassCurrR_;

	// Sound thread.
	aptk::LowpassRes< BcReal, BcReal >	LowPassL_;
	aptk::LowpassRes< BcReal, BcReal >	LowPassR_;
	aptk::PeakEQ< BcReal, BcReal >		PeakL_;
	aptk::PeakEQ< BcReal, BcReal >		PeakR_;
};

#endif

