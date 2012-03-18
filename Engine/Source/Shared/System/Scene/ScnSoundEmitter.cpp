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

#include "System/Content/CsCore.h"
#include "System/Sound/SsCore.h"

#ifdef PSY_SERVER
#include "Base/BcFile.h"
#include "Base/BcStream.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnSoundEmitter );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnSoundEmitter::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "ScnSoundEmitter" )
		//.field( "source",					csPVT_FILE,			csPCT_VALUE )
	.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnSoundEmitter::initialise()
{
	
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnSoundEmitter::create()
{
	Position_ = BcVec3d( 0.0f, 0.0f, 0.0f );
	Gain_ = 1.0f;
	Pitch_ = 1.0f;
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnSoundEmitter::destroy()
{
	BcAssertMsg( ChannelSoundMap_.size() == 0, "Sounds still playing on ScnSoundEmitter whilst being destroyed! Reference count mismatch!" );
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnSoundEmitter::isReady()
{
	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// play
void ScnSoundEmitter::play( ScnSoundRef Sound )
{
	// Acquire before playing (callback is threaded)
	CsResource::acquire();
	
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
		CsResource::release();
	}
}

//////////////////////////////////////////////////////////////////////////
// stopAll
void ScnSoundEmitter::stopAll()
{
	// Stop all bound channels.
	for( TChannelSoundMapIterator It( ChannelSoundMap_.begin() ); It != ChannelSoundMap_.end(); ++It )
	{
		// Stop channel.
		(*It).first->stop();
	}
}

//////////////////////////////////////////////////////////////////////////
// setPosition
void ScnSoundEmitter::setPosition( const BcVec3d& Position )
{
	Position_ = Position;
}

//////////////////////////////////////////////////////////////////////////
// setGain
void ScnSoundEmitter::setGain( BcReal Gain )
{
	Gain_ = Gain;
}

//////////////////////////////////////////////////////////////////////////
// setPitch
void ScnSoundEmitter::setPitch( BcReal Pitch )
{
	Pitch_ = Pitch;
}

//////////////////////////////////////////////////////////////////////////
// onStarted
//virtual
void ScnSoundEmitter::onStarted( SsChannel* pSound )
{
	onPlaying( pSound );
}

//////////////////////////////////////////////////////////////////////////
// onPlaying
//virtual
void ScnSoundEmitter::onPlaying( SsChannel* pSound )
{
	// Update parameters.
	pSound->gain( Gain_ );
	//pSound->maxDistance( 1.0f );
	//pSound->maxDistance( 1.0f );
	pSound->rolloffFactor( 0.05f );
	pSound->position( Position_ );
	pSound->pitch( Pitch_ );
}

//////////////////////////////////////////////////////////////////////////
// onEnded
//virtual
void ScnSoundEmitter::onEnded( SsChannel* pSound )
{
	// Find in map, and erase reference.
	TChannelSoundMapIterator Iter = ChannelSoundMap_.find( pSound );
	
	if( Iter != ChannelSoundMap_.end() )
	{
		ChannelSoundMap_.erase( Iter );

		CsResource::release();
	}
}
