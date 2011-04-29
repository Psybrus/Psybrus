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

#include "ScnSoundEmitter.h"

#include "CsCore.h"
#include "SsCore.h"

#ifdef PSY_SERVER
#include "BcFile.h"
#include "BcStream.h"
#include "json.h"
#endif


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnSoundEmitter );

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
	
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnSoundEmitter::destroy()
{
	
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
	SsChannel* pChannel = SsCore::pImpl()->play( pSample, this );

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
	pSound->gain( 1.0f );
	pSound->minDistance( 1.0f );
	pSound->rolloffFactor( 0.05f );
	pSound->position( BcVec3d( 0.0f, 0.0f, 0.0f ) );
}

//////////////////////////////////////////////////////////////////////////
// onEnded
//virtual
void ScnSoundEmitter::onEnded( SsChannel* pSound )
{
	// Release ourself, no longer in use.
	CsResource::release();

	// Find in map, and erase reference.
	TChannelSoundMapIterator Iter = ChannelSoundMap_.find( pSound );
	
	if( Iter != ChannelSoundMap_.end() )
	{
		ChannelSoundMap_.erase( Iter );
	}
}
