/**************************************************************************
*
* File:		GaPortaudioComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	15/12/12		
* Description:
*		Portaudio component.
*		
*
*
* 
**************************************************************************/

#include "GaPortaudioComponent.h"

//////////////////////////////////////////////////////////////////////////
// Statics.
BcAtomicU32 GaPortaudioComponent::GlobalPaRefCount_;

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaPortaudioComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaPortaudioComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	pPaStream_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaPortaudioComponent::update( BcReal Tick )
{
	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaPortaudioComponent::onAttach( ScnEntityWeakRef Parent )
{
	BcAssertMsg( pPaStream_ == NULL, "Already opened stream." );

	if( GlobalPaRefCount_++ == 0 )
	{
		Pa_Initialize();
	}

	PaError Error = Pa_OpenDefaultStream( &pPaStream_, 1, 0, paFloat32, 44100.0, 2048, &GaPortaudioComponent::StaticStreamCallback, this );
	BcAssertMsg( Error == paNoError, "Can't init port audio." );

	Pa_StartStream( pPaStream_ );

	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaPortaudioComponent::onDetach( ScnEntityWeakRef Parent )
{
	Pa_StopStream( pPaStream_ );
	Pa_CloseStream( pPaStream_ );
	pPaStream_ = NULL;

	if( --GlobalPaRefCount_ == 0 )
	{
		Pa_Terminate();
	}

	// Don't forget to detach!
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// noofInputFrames
BcU32 GaPortaudioComponent::noofInputFrames() const
{
	BcScopedLock< BcMutex > Lock( InputBufferLock_ );
	return InputBuffer_.size();
}

//////////////////////////////////////////////////////////////////////////
// popInputFrames
void GaPortaudioComponent::popInputFrames( BcU32 NoofFrames, std::vector< BcF32 >& OutputBuffer )
{
	BcScopedLock< BcMutex > Lock( InputBufferLock_ );
	BcAssert( noofInputFrames() >= NoofFrames );
	OutputBuffer.reserve( OutputBuffer.size() + NoofFrames );

	for( BcU32 Idx = 0; Idx < NoofFrames; ++Idx )
	{
		OutputBuffer.push_back( InputBuffer_[ Idx ] );
	}

	InputBuffer_.erase( InputBuffer_.begin(), InputBuffer_.begin() + NoofFrames );
}

//////////////////////////////////////////////////////////////////////////
// StaticStreamCallback
//static
int GaPortaudioComponent::StaticStreamCallback(
		const void *input, void *output,
		unsigned long frameCount,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData )
{
	GaPortaudioComponent* pPortaudioComponent = reinterpret_cast< GaPortaudioComponent* >( userData );
	return pPortaudioComponent->streamCallback( input, output, frameCount, timeInfo, statusFlags );
}

//////////////////////////////////////////////////////////////////////////
// streamCallback
int GaPortaudioComponent::streamCallback(
		const void *input, void *output,
		unsigned long frameCount,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags )
{
	BcScopedLock< BcMutex > Lock( InputBufferLock_ );

	// Round up total size and buffer.
	BcU32 RoundedUpTotalSize = BcPotNext( InputBuffer_.size() + frameCount );
	InputBuffer_.reserve( RoundedUpTotalSize );

	// Copy frames in for processing by the game, rather than use a game thread dispatch.
	const BcF32* pInputF32 = reinterpret_cast< const BcF32* >( input );
	for( BcU32 Idx = 0; Idx < frameCount; ++Idx )
	{
		InputBuffer_.push_back( *pInputF32++ );
	}
	
	return 0;
}
