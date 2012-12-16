/**************************************************************************
*
* File:		GaPlayerComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	15/12/12		
* Description:
*		Player component.
*		
*
*
* 
**************************************************************************/

#include "GaPlayerComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
#if PSY_DEBUG
const BcU32 GBufferProcessingSize = 512;
#else
const BcU32 GBufferProcessingSize = 1024;
#endif

const BcReal GWidthPitch = 300.0f;
const BcReal GMiddlePitch = 1500.0f;

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaPlayerComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaPlayerComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	SmoothedEstimatedPitch_ = 0.0f;

	LockEstimatedPitch_ = 0.0f;
	MaxLockTime_ = 200.0f / 1000.0f;
	MaxCooldownTime_ = 100.0f / 1000.0f;
	LockTimer_ = MaxLockTime_;
	LockCooldownTimer_= MaxCooldownTime_;

	PitchLocked_ = 0.0f;
	NoiseLocked_ = 0.0f;

}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaPlayerComponent::update( BcReal Tick )
{
	Super::update( Tick );

	// Do pitch detection.
	BcReal EstimatedPitch = 0.0f;
	BcReal PeriodSD = 0.0f;
	BcReal RMS = 0.0f;
	BcBool IsLoudEnoughToAccept = BcFalse;
	BcBool GotPitchLock = BcFalse;
	BcBool GotNoiseLock = BcFalse;
	
	// Grab and process the buffers.
	BcReal AudioTick = 0.0f;
	while( PortaudioComponent_->noofInputFrames() > GBufferProcessingSize )
	{
		AudioTick += static_cast< BcReal> ( GBufferProcessingSize ) / 44100.0f;

		InputBuffer_.clear();
		PortaudioComponent_->popInputFrames( GBufferProcessingSize, InputBuffer_ );
	}

	{
		// Autocorrelate.
		autoCorrelateInputBuffer();

		// Find the peaks in autocorrelation.
		findAutocorrelationPeaks();

		// Analyze audio.
		analyzeAudio( EstimatedPitch, PeriodSD, RMS );

		if( RMS > 0.05f )
		{
			IsLoudEnoughToAccept = BcTrue;
		}

		// Got a lock on pitch.
		if( IsLoudEnoughToAccept )
		{
			if( PeriodSD < 32.0f )
			{
				SmoothedEstimatedPitch_ = ( 0.1f * EstimatedPitch ) + ( 0.9f * SmoothedEstimatedPitch_ );
				GotPitchLock = BcTrue;
			}
			else if ( PeriodSD > 64.0f )
			{
				GotNoiseLock = BcTrue;
			}

			LockTimer_ -= AudioTick;
			LockCooldownTimer_ = MaxCooldownTime_;

			if( GotPitchLock )
			{
				PitchLocked_ += AudioTick;
			}

			if( GotNoiseLock )
			{
				NoiseLocked_ += AudioTick;
			}

			if( LockTimer_ < 0.0f )
			{
				if( PitchLocked_ >= NoiseLocked_ )
				{
					onPitchLock( SmoothedEstimatedPitch_ );
				}
				else
				{
					onNoiseLock();
				}

				PitchLocked_ = 0.0f;
				NoiseLocked_ = 0.0f;
				LockTimer_ = MaxCooldownTime_;
			}
		}
		else
		{
			LockCooldownTimer_ -= AudioTick;
			if( LockCooldownTimer_ < 0.0f )
			{
				LockTimer_ = MaxLockTime_;
			}
		}
	}
	
	// Debug render waveform.
	Canvas_->clear();
	ScnMaterialComponentRef MaterialComponent( getParentEntity()->getComponentByType<ScnMaterialComponent>( 0 ) );
	Canvas_->setMaterialComponent( MaterialComponent );

	OsClient* pClient = OsCore::pImpl()->getClient( 0 );
	BcReal HW = static_cast< BcReal >( pClient->getWidth() ) / 2.0f;
	BcReal HH = static_cast< BcReal >( pClient->getHeight() ) / 2.0f;
	BcReal AspectRatio = HW / HH;
		
	BcMat4d Ortho;
	Ortho.orthoProjection( -HW, HW, HH, -HH, -1.0f, 1.0f );
		
	Canvas_->pushMatrix( Ortho );

#if 0
	{
	
		BcReal IncrementOnX = ( 2.0f * HW ) / static_cast< BcReal >( InputBuffer_.size() );
		BcReal AccumulatorX;
		BcVec2d PrevPosition;
		BcVec2d CurrPosition;
		
		AccumulatorX = -HW;
		PrevPosition = BcVec2d( -HW, 0.0f );
		CurrPosition = BcVec2d( -HW, 0.0f );
		for( BcU32 Idx = 0; Idx < InputBuffer_.size(); ++Idx )
		{
			BcReal FrameValue = InputBuffer_[ Idx ];

			CurrPosition = BcVec2d( AccumulatorX, FrameValue * HH );
			Canvas_->drawLine( PrevPosition, CurrPosition, RsColour::GREEN, 0 );
			PrevPosition = CurrPosition;
			AccumulatorX += IncrementOnX;
		}

		AccumulatorX = -HW;
		PrevPosition = BcVec2d( -HW, 0.0f );
		CurrPosition = BcVec2d( -HW, 0.0f );
		for( BcU32 Idx = 0; Idx < InputBuffer_.size(); ++Idx )
		{
			BcReal FrameValue = AutocorrelationBuffer_[ Idx ];

			CurrPosition = BcVec2d( AccumulatorX, FrameValue );
			Canvas_->drawLine( PrevPosition, CurrPosition, RsColour::RED, 0 );
			PrevPosition = CurrPosition;
			AccumulatorX += IncrementOnX;
		}
		
		if( EstimatedPitch > 0.0f )
		{
			for( BcU32 Idx = 0; Idx < AutocorrelationPeaks_.size(); ++Idx )
			{
				TPeak Peak = AutocorrelationPeaks_[ Idx ];
				
				PrevPosition = BcVec2d( -HW + IncrementOnX * Peak.Index_, -1.0f + Peak.Value_ );
				CurrPosition = BcVec2d( -HW + IncrementOnX * Peak.Index_, 1.0f + Peak.Value_ );
				Canvas_->drawLine( PrevPosition, CurrPosition, RsColour::BLUE, 0 );
			}
		}
	}
#endif 

	// Movement.
	Position_ = ( Position_ * 0.975f ) + ( TargetPosition_ * 0.025f );

	// Hacky floaty.
	static BcReal Ticker = 0.0f;
	Ticker += Tick;
	BcMat4d Matrix;
	BcVec3d FinalPosition = Position_ + BcVec3d( BcSin( Ticker ) * 0.05f, 0.0f, BcCos( Ticker * 0.7f ) * 0.05f );
	Matrix.rotation( BcVec3d( BcSin( Ticker ) * 0.01f, BcCos( Ticker * 0.9f ) * 0.02f, 0.0f ) );
	Matrix.translation( FinalPosition );
	getParentEntity()->setMatrix( Matrix );

	particleEngine( BcVec3d( -0.1f, -0.18f, -0.05f ) );
	particleEngine( BcVec3d(  0.1f, -0.18f, -0.05f ) );

	BcChar Buffer[ 2048 ];
	BcSPrintf( Buffer, "Smoothed Estimated Pitch: %f\nEstimated Pitch: %f hz\nPeriod SD: %f\nRMS: %f\n%s\n%s\n%s", SmoothedEstimatedPitch_, EstimatedPitch, PeriodSD, RMS, 
		IsLoudEnoughToAccept ? "LOUD ENOUGH TO USE" : "",
		GotPitchLock ? "GOT PITCH LOCK" : "",
		GotNoiseLock ? "IS NOISE" : "" );
	Font_->draw( Canvas_, BcVec2d( -HW + 32.0f, -HH + 32.0f ), Buffer, RsColour::WHITE, BcFalse );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaPlayerComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Don't forget to attach!
	Super::onAttach( Parent );

	// Grab port audio component.
	PortaudioComponent_ = Parent->getComponentByType< GaPortaudioComponent >( 0 );
	
	// Grab the canvas.
	Canvas_ = Parent->getComponentAnyParentByType< ScnCanvasComponent >( 0 );

	// Grab the font.
	Font_ = Parent->getComponentAnyParentByType< ScnFontComponent >( 0 );

	// Grab particle system.
	ShipParticles_ = Parent->getComponentByType< ScnParticleSystemComponent >( 0 );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaPlayerComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Don't forget to detach!
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// autoCorrelateInputBuffer
void GaPlayerComponent::autoCorrelateInputBuffer()
{
	AutocorrelationBuffer_.clear();
	AutocorrelationBuffer_.reserve( GBufferProcessingSize );
	
	if( InputBuffer_.size() > 0 )
	{
		for( BcU32 IdxA = 0; IdxA < GBufferProcessingSize; ++IdxA )
		{
			BcReal Sum = 0.0f;
			for( BcU32 IdxB = 0; IdxB < GBufferProcessingSize - IdxA; ++IdxB )
			{
				Sum += InputBuffer_[ IdxB ] * InputBuffer_[ IdxB + IdxA ];
			}
			AutocorrelationBuffer_.push_back( Sum );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// findAutocorrelationPeaks
void GaPlayerComponent::findAutocorrelationPeaks()
{
	BcReal PrevGradient = 0.0f;

	AutocorrelationPeaks_.clear();
	AutocorrelationPeaks_.reserve( AutocorrelationBuffer_.size() );

	for( BcU32 Idx = 1; Idx < AutocorrelationBuffer_.size(); ++Idx )
	{
		BcReal PrevValue = AutocorrelationBuffer_[ Idx - 1 ];
		BcReal CurrValue = AutocorrelationBuffer_[ Idx ];
		BcReal CurrGradient = CurrValue - PrevValue;
		
		if( ( CurrGradient > 0.0f && PrevGradient < 0.0f ) ||
			( CurrGradient < 0.0f && PrevGradient > 0.0f ) )
		{
			TPeak Peak = 
			{
				static_cast< BcF32 >( Idx ),
				CurrValue
			};

			AutocorrelationPeaks_.push_back( Peak );
		}
		
		PrevGradient = CurrGradient;
	}

	// Sort them by size (+ve first, -ve second).
	std::sort( AutocorrelationPeaks_.begin(), AutocorrelationPeaks_.end(), GaPlayerComponentAutocorrelationPeakSorter() );
}

//////////////////////////////////////////////////////////////////////////
// analyzeAudio
void GaPlayerComponent::analyzeAudio( BcReal& Pitch, BcReal& PeriodSD, BcReal& RMS )
{
	if( AutocorrelationPeaks_.size() >= 2 )
	{
		// Simple distance between 2 largest peaks.
		TPeak PeakA = AutocorrelationPeaks_[ 0 ];
		TPeak PeakB = AutocorrelationPeaks_[ 1 ];

		BcReal PeriodDistance = BcAbs( PeakB.Index_ - PeakA.Index_ );

		// Find closely matching ones to calculate an average.
		BcReal MaxDifference = 2.0f;
		BcReal AveragePeriodDistance = PeriodDistance;
		BcReal TotalMatchingPeriods = 1.0f;
		BcReal RealAveragePeriodDistance = PeriodDistance;
		BcReal RealTotalMatchingPeriods = 1.0f;

		BcU32 TotalPeriods = AutocorrelationPeaks_.size() / 2;
		for( BcU32 Idx = 1; Idx < TotalPeriods; ++Idx )
		{
			TPeak PeakC = AutocorrelationPeaks_[ Idx * 2 ];
			TPeak PeakD = AutocorrelationPeaks_[ Idx * 2 + 1 ];

			BcReal ThisPeriodDistance = BcAbs( PeakD.Index_ - PeakC.Index_ );

			if( BcAbs( ThisPeriodDistance - PeriodDistance ) < MaxDifference )
			{
				AveragePeriodDistance += ThisPeriodDistance;
				TotalMatchingPeriods += 1.0f;
			}

			RealAveragePeriodDistance += ThisPeriodDistance;
			RealTotalMatchingPeriods += 1.0f;
		}

		AveragePeriodDistance /= TotalMatchingPeriods;
		RealAveragePeriodDistance /= RealTotalMatchingPeriods;

		// Calculate standard deviation of the periods so we can report lock accuracy.
		BcReal SumSD = 0.0f;
		for( BcU32 Idx = 1; Idx < TotalPeriods; ++Idx )
		{
			TPeak PeakC = AutocorrelationPeaks_[ Idx * 2 ];
			TPeak PeakD = AutocorrelationPeaks_[ Idx * 2 + 1 ];

			BcReal ThisPeriodDistance = BcAbs( PeakD.Index_ - PeakC.Index_ );
			
			SumSD += ( ThisPeriodDistance - RealAveragePeriodDistance ) * ( ThisPeriodDistance - RealAveragePeriodDistance );
		}

		Pitch = 44100.0f / AveragePeriodDistance;
		PeriodSD = BcSqrt( SumSD * ( 1.0f / ( TotalPeriods - 1 ) ) );
	}
	else
	{
		Pitch = 0.0f;
		PeriodSD = 1e6f;
	}

	// Calculate RMS.
	BcReal SquaredTotal = 0.0f;
	for( BcU32 Idx = 0; Idx < InputBuffer_.size(); ++Idx )
	{
		BcReal FrameValue = InputBuffer_[ Idx ];
		SquaredTotal += FrameValue * FrameValue;
	}

	RMS = BcSqrt( SquaredTotal / static_cast< BcReal >( InputBuffer_.size() ) );
}

//////////////////////////////////////////////////////////////////////////
// onPitchLock
void GaPlayerComponent::onPitchLock( BcReal Pitch )
{
	BcPrintf( "GOT PITCH: %f\n", Pitch );

	const BcReal Width = 1.5f;
	BcVec3d LeftMost( -Width, 0.0f, 0.0f );
	BcVec3d RightMost( Width, 0.0f, 0.0f );

	Pitch = ( Pitch - ( GMiddlePitch - GWidthPitch ) ) / ( GWidthPitch * 2 );
	Pitch = BcClamp( Pitch, 0.0f, 1.0f );

	TargetPosition_.lerp( LeftMost, RightMost, Pitch );
}

//////////////////////////////////////////////////////////////////////////
// onNoiseLock
void GaPlayerComponent::onNoiseLock()
{
	BcPrintf( "GOT NOISE\n" );

	particleLaser( BcVec3d( -0.1f, 0.18f, -0.14f ) );
	particleLaser( BcVec3d(  0.1f, 0.18f, -0.14f ) );
}

//////////////////////////////////////////////////////////////////////////
// particleEngine
void GaPlayerComponent::particleEngine( BcVec3d Position )
{
	for( BcU32 Idx = 0; Idx < 12; ++Idx )
	{
		ScnParticle* pParticle = NULL;
		if( ShipParticles_->allocParticle( pParticle ) )
		{
			pParticle->Position_ = Position;
			pParticle->Velocity_ = BcVec3d( 0.0f, -0.5f, 0.0f ) + ( BcRandom::Global.randVec3Normal() * 0.05f );
			pParticle->Acceleration_ = BcVec3d( 0.0f, 0.0f, 0.0f );

			pParticle->Scale_ = BcVec2d( 0.1f, 0.1f );
			pParticle->MinScale_ = BcVec2d( 0.075f, 0.075f );
			pParticle->MaxScale_ = BcVec2d( 0.0f, 0.0f );

			pParticle->Rotation_ = BcRandom::Global.randReal();
			pParticle->RotationMultiplier_ = BcRandom::Global.randReal();

			pParticle->Colour_ = RsColour( 0.0f, 0.2f, 0.5f, 1.0f );
			pParticle->MinColour_ = RsColour( 0.0f, 0.2f, 0.5f, 1.0f );
			pParticle->MaxColour_ = RsColour( 1.0f, 1.0f, 0.0f, 0.0f );

			pParticle->TextureIndex_ = 1;
			pParticle->CurrentTime_ = 0.0f;
			pParticle->MaxTime_ = 0.25f;
			pParticle->Alive_ = BcTrue;		
		}
	}

	for( BcU32 Idx = 0; Idx < 1; ++Idx )
	{
		ScnParticle* pParticle = NULL;
		if( ShipParticles_->allocParticle( pParticle ) )
		{
			pParticle->Position_ = Position;
			pParticle->Velocity_ = BcVec3d( 0.0f, -0.3f, 0.0f );
			pParticle->Acceleration_ = BcVec3d( 0.0f, 0.0f, 0.0f );

			pParticle->Scale_ = BcVec2d( 0.1f, 0.1f );
			pParticle->MinScale_ = BcVec2d( 0.02f, 0.02f );
			pParticle->MaxScale_ = BcVec2d( 0.02f, 0.02f );

			pParticle->Rotation_ = BcRandom::Global.randReal();
			pParticle->RotationMultiplier_ = BcRandom::Global.randReal();

			pParticle->Colour_ = RsColour( 0.0f, 0.2f, 0.5f, 1.0f );
			pParticle->MinColour_ = RsColour( 0.0f, 0.0f, 1.0f, 1.0f );
			pParticle->MaxColour_ = RsColour( 0.0f, 0.0f, 0.0f, 0.0f );

			pParticle->TextureIndex_ = 1;
			pParticle->CurrentTime_ = 0.0f;
			pParticle->MaxTime_ = 2.5f;
			pParticle->Alive_ = BcTrue;		
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// particleEngine
void GaPlayerComponent::particleLaser( BcVec3d Position )
{
	BcVec3d PositionIncrement( 0.0f, 0.02f, 0.0f );

	for( BcU32 Idx = 0; Idx < 256; ++Idx )
	{
		ScnParticle* pParticle = NULL;
		if( ShipParticles_->allocParticle( pParticle ) )
		{
			pParticle->Position_ = Position;
			pParticle->Velocity_ = BcVec3d( 0.0f, 0.0f, 0.0f );
			pParticle->Acceleration_ = BcVec3d( 0.0f, 0.0f, 0.0f );

			pParticle->Scale_ = BcVec2d( 0.025f, 0.025f );
			pParticle->MinScale_ = BcVec2d( 0.025f, 0.025f );
			pParticle->MaxScale_ = BcVec2d( 0.0f, 0.0f );

			pParticle->Rotation_ = BcRandom::Global.randReal();
			pParticle->RotationMultiplier_ = BcRandom::Global.randReal();

			pParticle->Colour_ = RsColour( 0.5f, 0.01f, 0.01f, 1.0f );
			pParticle->MinColour_ = RsColour( 0.5f, 0.01f, 0.01f, 1.0f );
			pParticle->MaxColour_ = RsColour( 0.0f, 0.0f, 0.0f, 0.0f );

			pParticle->TextureIndex_ = 1;
			pParticle->CurrentTime_ = 0.0f;
			pParticle->MaxTime_ = 0.1f;
			pParticle->Alive_ = BcTrue;		
		}

		Position += PositionIncrement;
	}

}
