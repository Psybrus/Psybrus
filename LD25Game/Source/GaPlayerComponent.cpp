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
const BcU32 GBufferProcessingSize = 512;

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaPlayerComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaPlayerComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );
}

//////////////////////////////////////////////////////////////////////////
// GaPlayerComponent
//virtual
void GaPlayerComponent::update( BcReal Tick )
{
	Super::update( Tick );

	// Grab and process the buffers.
	while( PortaudioComponent_->noofInputFrames() > GBufferProcessingSize )
	{
		InputBuffer_.clear();
		PortaudioComponent_->popInputFrames( GBufferProcessingSize, InputBuffer_ );
	}

	// Autocorrelate.
	autoCorrelateInputBuffer();

	// Find the peaks in autocorrelation.
	findAutocorrelationPeaks();
	
	// Debug render waveform.
	{
		Canvas_->clear();
        ScnMaterialComponentRef MaterialComponent( getParentEntity()->getComponentByType<ScnMaterialComponent>( 0 ) );
        Canvas_->setMaterialComponent( MaterialComponent );

        OsClient* pClient = OsCore::pImpl()->getClient( 0 );
        BcReal HW = 768.0f / 32.0f; // static_cast< BcReal >( pClient->getWidth() ) / 2.0f;
        BcReal HH = 432.0f / 32.0f; // static_cast< BcReal >( pClient->getHeight() ) / 2.0f;
        BcReal AspectRatio = HW / HH;
		
        BcMat4d Ortho;
        Ortho.orthoProjection( -HW, HW, HH, -HH, -1.0f, 1.0f );
		
        Canvas_->pushMatrix( Ortho );
		
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

		if( AutocorrelationPeaks_.size() >= 2 )
		{
			for( BcU32 Idx = 0; Idx < 2; ++Idx )
			{
				TPeak Peak = AutocorrelationPeaks_[ Idx ];
				
				PrevPosition = BcVec2d( -HW + IncrementOnX * Peak.Index_, -120.0f );
				CurrPosition = BcVec2d( -HW + IncrementOnX * Peak.Index_, 120.0f );
				Canvas_->drawLine( PrevPosition, CurrPosition, RsColour::BLUE, 0 );
				
			}

			BcReal EstimatedPitch = estimatePitch();

			BcChar Buffer[ 2048 ];
			BcSPrintf( Buffer, "Estimated Pitch: %f hz", EstimatedPitch );

			BcMat4d ScaleMatrix;
			ScaleMatrix.scale( BcVec3d( 0.1f, 0.1f, 0.1f ) );

			Canvas_->pushMatrix( ScaleMatrix );

			Font_->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), Buffer, RsColour::WHITE, BcFalse );

			Canvas_->popMatrix();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// GaPlayerComponent
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
}

//////////////////////////////////////////////////////////////////////////
// GaPlayerComponent
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
// estimatePitch
BcReal GaPlayerComponent::estimatePitch()
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
		}

		AveragePeriodDistance /= TotalMatchingPeriods;

		return 44100.0f / AveragePeriodDistance;
	}

	return 0.0f;
}