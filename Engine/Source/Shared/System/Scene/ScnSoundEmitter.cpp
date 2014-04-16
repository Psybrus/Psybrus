/**************************************************************************
*
* File:		ScnSoundEmitter.cpp
* Author:	Neil Richardson 
* Ver/Date:	28/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/ScnSoundEmitter.h"

#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"
#include "System/Sound/SsCore.h"

#include "Base/BcRandom.h"

#ifdef PSY_SERVER
#include "Base/BcFile.h"
#include "Base/BcStream.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnSoundEmitterComponent );

void ScnSoundEmitterComponent::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "Position_",			&ScnSoundEmitterComponent::Position_ ),
		ReField( "Gain_",				&ScnSoundEmitterComponent::Gain_ ),
		ReField( "Pitch_",				&ScnSoundEmitterComponent::Pitch_ ),
	};
		
	ReRegisterClass< ScnSoundEmitterComponent, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnSoundEmitterComponent::initialise( const Json::Value& Object )
{
	Super::initialise();

	Position_ = BcVec3d( 0.0f, 0.0f, 0.0f );
	Gain_ = 1.0f;
	Pitch_ = 1.0f;	
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnSoundEmitterComponent::create()
{
	Super::create();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnSoundEmitterComponent::destroy()
{
	if( SsCore::pImpl() )
	{
		SsCore::pImpl()->unregister( this );
	}

	Super::destroy();
}

//////////////////////////////////////////////////////////////////////////
// play
void ScnSoundEmitterComponent::play( ScnSoundRef Sound )
{
	// Acquire before playing (callback is threaded)
	//CsResource::acquire();
	
	// Get sample from sound.
	SsSample* pSample = Sound->getSample();

	// Play sample.
	SsChannel* pChannel = SsCore::pImpl() != NULL ? SsCore::pImpl()->play( pSample, this ) : NULL;

	// Add to map, or release if not played.
	if( pChannel != NULL )
	{
		ChannelSoundMap_[ pChannel ] = Sound;
	}
	else
	{
		//CsResource::release();
	}
}

//////////////////////////////////////////////////////////////////////////
// stopAll
void ScnSoundEmitterComponent::stopAll()
{
	// Stop all bound channels.
	for( TChannelSoundMapIterator It( ChannelSoundMap_.begin() ); It != ChannelSoundMap_.end(); ++It )
	{
		// Stop channel.
		(*It).first->stop();
	}
}

//////////////////////////////////////////////////////////////////////////
// setGain
void ScnSoundEmitterComponent::setGain( BcF32 Gain )
{
	Gain_ = Gain;
}

//////////////////////////////////////////////////////////////////////////
// setPitch
void ScnSoundEmitterComponent::setPitch( BcF32 Pitch )
{
	Pitch_ = Pitch;
}

//////////////////////////////////////////////////////////////////////////
// setPitch
//virtual
void ScnSoundEmitterComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// setPitch
//virtual
void ScnSoundEmitterComponent::onDetach( ScnEntityWeakRef Parent )
{
	stopAll();

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onStarted
//virtual
void ScnSoundEmitterComponent::onStarted( SsChannel* pSound )
{
	if( isAttached() )
	{
		onPlaying( pSound );
	}
}

//////////////////////////////////////////////////////////////////////////
// onPlaying
//virtual
void ScnSoundEmitterComponent::onPlaying( SsChannel* pSound )
{
	if( isAttached() )
	{
		// Update parameters.
		pSound->gain( Gain_ );
		//pSound->maxDistance( 1.0f );
		//pSound->maxDistance( 1.0f );
		pSound->rolloffFactor( 0.05f );
		pSound->position( getParentEntity()->getWorldMatrix().translation() );
		pSound->pitch( Pitch_ );
	}
}

//////////////////////////////////////////////////////////////////////////
// onEnded
//virtual
void ScnSoundEmitterComponent::onEnded( SsChannel* pSound )
{
	// Find in map, and erase reference.
	TChannelSoundMapIterator Iter = ChannelSoundMap_.find( pSound );
	
	if( Iter != ChannelSoundMap_.end() )
	{
		ChannelSoundMap_.erase( Iter );

		//CsResource::release();
	}
}
