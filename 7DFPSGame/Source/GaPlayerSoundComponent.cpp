/**************************************************************************
*
* File:		GaPlayerSoundComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		Player component.
*		
*
*
* 
**************************************************************************/

#include "GaPlayerSoundComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaPlayerSoundComponent );

BCREFLECTION_EMPTY_REGISTER( GaPlayerSoundComponent );
/*
BCREFLECTION_DERIVED_BEGIN( ScnComponent, GaPlayerSoundComponent )
	BCREFLECTION_MEMBER( BcName,							Name_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								Index_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( CsPackage,							pPackage_,						bcRFF_POINTER | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								RefCount_,						bcRFF_DEFAULT | bcRFF_TRANSIENT ),
BCREFLECTION_DERIVED_END();
*/

//////////////////////////////////////////////////////////////////////////
// PortAudio init/deinit.
static BcU32 PortAudioRefs = 0;
static void acquirePortAudio()
{
	if( PortAudioRefs == 0 )
	{
		Pa_Initialize();
	}	
	++PortAudioRefs;
}

static void releasePortAudio()
{
	--PortAudioRefs;
	if( PortAudioRefs == 0 )
	{
		Pa_Terminate();
	}
}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaPlayerSoundComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	SampleRate_ = 22050;
	BufferSize_ = 1024 * 2;

	FilterLock_.lock();
	LowPassCurrL_.setup( (BcF32)SampleRate_ / 4.0f, 0.0f, (BcF32)SampleRate_ );
	LowPassCurrR_.setup( (BcF32)SampleRate_ / 4.0f, 0.0f, (BcF32)SampleRate_ );
	LowPassPrevL_ = LowPassCurrL_;
	LowPassPrevR_ = LowPassCurrR_;
	FilterLock_.unlock();
	DoneReset_ = 8;
}

//////////////////////////////////////////////////////////////////////////
// GaPlayerSoundComponent
//virtual
void GaPlayerSoundComponent::update( BcF32 Tick )
{
	// Consts.
	const BcF32 MinCutoff = 100.0f;
	const BcF32 MaxCutoff = 1500.0f;
	const BcF32 MinRes = 0.0f;
	const BcF32 MaxRes = 0.9f;
	const BcF32 MinDistance = 2.0f;
	const BcF32 MaxDistance = 20.0f;
	
	// Send out rays, calc avg. distance.
	BcF32 DistanceL = 0.0f;
	BcF32 DistanceR = 0.0f;
	BcF32 MinDistanceL = MaxDistance;
	BcF32 MinDistanceR = MaxDistance;
	BcF32 MaxDistanceL = 0.0f;
	BcF32 MaxDistanceR = 0.0f;
	for( BcU32 Idx = 0; Idx < 4; ++Idx )
	{
		BcVec3d RayVectorL = Player_->EarLVectors_[ Idx ];
		BcVec3d RayVectorR = Player_->EarRVectors_[ Idx ];
	
		BcVec3d IntersectionL = intersection( RayVectorL );
		BcVec3d IntersectionR = intersection( RayVectorR );
		DistanceL += ( IntersectionL - getParentEntity()->getPosition() ).magnitude();
		DistanceR += ( IntersectionR - getParentEntity()->getPosition() ).magnitude();

		MinDistanceL = BcMin( MinDistanceL, DistanceL );
		MinDistanceR = BcMin( MinDistanceR, DistanceR );
		MaxDistanceL = BcMax( MaxDistanceL, DistanceL );
		MaxDistanceR = BcMax( MaxDistanceR, DistanceR );
	}

	DistanceL /= 4.0f;
	DistanceR /= 4.0f;

	// Setup cutoffs.
	BcF32 CutoffLerpL = BcClamp( ( DistanceL - MinDistance ) / MaxDistance, 0.0f, 1.0f );
	BcF32 CutoffLerpR = BcClamp( ( DistanceR - MinDistance ) / MaxDistance, 0.0f, 1.0f );
	BcF32 ResLerpL = BcClamp( ( MaxDistanceL - MinDistanceL ) / MaxDistance, 0.0f, 1.0f );
	BcF32 ResLerpR = BcClamp( ( MaxDistanceR - MinDistanceR ) / MaxDistance, 0.0f, 1.0f );
	
	BcF32 CutoffL = BcLerp( MinCutoff, MaxCutoff, CutoffLerpL );
	BcF32 CutoffR = BcLerp( MinCutoff, MaxCutoff, CutoffLerpR );
	BcF32 ResL = BcLerp( MinRes, MaxRes, CutoffLerpL );
	BcF32 ResR = BcLerp( MinRes, MaxRes, CutoffLerpR );
	
	// Setup new filter stuff.
	if( DoneReset_ == 0 )
	{
		FilterLock_.lock();
		static BcF32 Ticker = 0.0f;
		Ticker += Tick;
		LowPassCurrL_.setup( CutoffL, ResL, (BcF32)SampleRate_ );
		LowPassCurrR_.setup( CutoffR, ResR, (BcF32)SampleRate_ );
		FilterLock_.unlock();
	}
	else
	{
		--DoneReset_;
	}
	
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaPlayerSoundComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Don't forget to attach!
	Super::onAttach( Parent );
	
	// Cache world stuff we need.
	ScnEntityRef WorldEntity = ScnCore::pImpl()->findEntity( "WorldEntity_0" );
	Canvas_ = WorldEntity->getComponentByType< ScnCanvasComponent >( 0 );
	BSP_ = WorldEntity->getComponentByType< GaWorldBSPComponent >( 0 );
	Pressure_ = WorldEntity->getComponentByType< GaWorldPressureComponent >( 0 );

	// Subscribe to reset event.
	GaWorldResetEvent::Delegate OnResetEvent = GaWorldResetEvent::Delegate::bind< GaPlayerSoundComponent, &GaPlayerSoundComponent::onReset >( this );
	WorldEntity->subscribe( gaEVT_CORE_RESET, OnResetEvent );

	// Grab pawn.
	Pawn_ = Parent->getComponentByType< GaPawnComponent >( 0 );

	// Grab player.
	Player_ = Parent->getComponentByType< GaPlayerComponent >( 0 );

	//
	acquirePortAudio();

	// enum devices.
	PaDeviceIndex NoofDevice = Pa_GetDeviceCount();
	PaDeviceIndex DefaultOutputDevice = Pa_GetDefaultOutputDevice();
	
	// Create stream.
	PaError Error = Pa_OpenDefaultStream( &pPaStream_,
								          0,
								          2,
										  paFloat32,
								          SampleRate_,
								          BufferSize_,
								          &GaPlayerSoundComponent::streamCallback,
								          this );
	BcAssert( Error == paNoError );

	pWorkingBuffers_[ 0 ] = new BcF32[ BufferSize_ ];
	pWorkingBuffers_[ 1 ] = new BcF32[ BufferSize_ ];
	
	Pa_StartStream( pPaStream_ );

}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaPlayerSoundComponent::onDetach( ScnEntityWeakRef Parent )
{
	Canvas_ = NULL;
	BSP_ = NULL;
	Pressure_ = NULL;
	Pawn_ = NULL;
	Player_ = NULL;

	Pa_StopStream( pPaStream_ );
	Pa_CloseStream( pPaStream_ );
	pPaStream_ = NULL;

	// Release portaudio.
	releasePortAudio();

	delete [] pWorkingBuffers_[ 0 ];
	delete [] pWorkingBuffers_[ 1 ];
	pWorkingBuffers_[ 0 ] = NULL;
	pWorkingBuffers_[ 1 ] = NULL;

	ScnEntityRef WorldEntity = ScnCore::pImpl()->findEntity( "WorldEntity_0" );
	WorldEntity->unsubscribeAll( this );


	// Don't forget to detach!
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// intersection
BcVec3d GaPlayerSoundComponent::intersection( const BcVec3d& Direction )
{
	BcVec3d Position = getParentEntity()->getPosition();
	BcVec3d Target = Position + Direction * 256.0f;
	BcBSPPointInfo BSPPointInfo;
	if( BSP_->lineIntersection( Position, Target, &BSPPointInfo ) )
	{
		int a = 0; ++a;
	}
	else
	{
		BcPlane Floor( BcVec3d( 0.0f, 0.0f,  1.0f ), 4.0f );
		BcPlane Ceil( BcVec3d( 0.0f, 0.0f, -1.0f ), 4.0f );
		BcF32 Dist;
		Floor.lineIntersection( Position, Target, Dist, BSPPointInfo.Point_ );
		Ceil.lineIntersection( Position, Target, Dist, BSPPointInfo.Point_ );
	}


	if( BSP_->InEditorMode_ )
	{
		Canvas_->drawLine( BcVec2d( Position.x(), Position.y() ), BcVec2d( BSPPointInfo.Point_.x(), BSPPointInfo.Point_.y() ), RsColour::GREEN, 10 );
	}

	return BSPPointInfo.Point_;
}

//////////////////////////////////////////////////////////////////////////
// streamCallback
int GaPlayerSoundComponent::streamCallback( const void *input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData )
{
	GaPlayerSoundComponent* pSynthesizer = (GaPlayerSoundComponent*)userData;
	BcF32* pOutput = (BcF32*)output;
	pSynthesizer->process( frameCount, pOutput );
	return paContinue;
}

//////////////////////////////////////////////////////////////////////////
// process
void GaPlayerSoundComponent::process( BcU32 NoofFrames, BcF32* pFrames )
{
	BcAssert( NoofFrames <= BufferSize_ );

	BcF32* pBufferL = pWorkingBuffers_[ 0 ];
	BcF32* pBufferR = pWorkingBuffers_[ 1 ];

	// Fill with equal levels of noise.
	for( BcU32 Idx = 0; Idx < NoofFrames; ++Idx )
	{
		BcF32 Value = NoiseGenerator_.randReal();
		pBufferL[ Idx ] = Value * 0.25f;
		pBufferR[ Idx ] = Value * 0.25f;
	}

	// Setup low pass filters.
	aptk::LowpassRes< BcF32, BcF32 > LowPassPrevL;
	aptk::LowpassRes< BcF32, BcF32 > LowPassPrevR;
	aptk::LowpassRes< BcF32, BcF32 > LowPassCurrL;
	aptk::LowpassRes< BcF32, BcF32 > LowPassCurrR;

	// Copy over the filter data.
	FilterLock_.lock();
	LowPassPrevL = LowPassPrevL_;
	LowPassPrevR = LowPassPrevR_;
	LowPassCurrL = LowPassCurrL_;
	LowPassCurrR = LowPassCurrR_;

	// Set previous for next time round.
	LowPassPrevL_ = LowPassCurrL_;
	LowPassPrevR_ = LowPassCurrR_;

	FilterLock_.unlock();

	BcF32 LerpValue = 0.0f;
	BcF32 LerpStep = 1.0f / (BcF32)NoofFrames;

	// Do lowpass.
	for( BcU32 Idx = 0; Idx < NoofFrames; ++Idx )
	{
		// Interpolate the low passes.
		LowPassL_.lerp( LowPassPrevL, LowPassCurrL, LerpValue );
		LowPassR_.lerp( LowPassPrevR, LowPassCurrR, LerpValue );
		LerpValue += LerpStep;
		
		pBufferL[ Idx ] = LowPassL_.procData( pBufferL[ Idx ] );
		pBufferR[ Idx ] = LowPassR_.procData( pBufferR[ Idx ] );
	}

	// Write to frame.
	for( BcU32 Idx = 0; Idx < NoofFrames; ++Idx )
	{
		*pFrames++ = *pBufferL++;
		*pFrames++ = *pBufferR++;
	}
}

//////////////////////////////////////////////////////////////////////////
// onReset
eEvtReturn GaPlayerSoundComponent::onReset( EvtID ID, const GaWorldResetEvent& Event )
{
	FilterLock_.lock();
	LowPassCurrL_.setup( (BcF32)SampleRate_ / 4.0f, 0.0f, (BcF32)SampleRate_ );
	LowPassCurrR_.setup( (BcF32)SampleRate_ / 4.0f, 0.0f, (BcF32)SampleRate_ );
	FilterLock_.unlock();
	DoneReset_ = 8;

	return evtRET_PASS;
}
