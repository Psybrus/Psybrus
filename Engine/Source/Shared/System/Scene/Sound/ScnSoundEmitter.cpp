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

#include "System/Scene/Sound/ScnSoundEmitter.h"

#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"
#include "System/Sound/SsCore.h"

#include "Base/BcRandom.h"

#ifdef PSY_IMPORT_PIPELINE
#include "Base/BcFile.h"
#include "Base/BcStream.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnSoundEmitterComponent );

void ScnSoundEmitterComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Params_", &ScnSoundEmitterComponent::Params_ ),
	};
		
	ReRegisterClass< ScnSoundEmitterComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( -2010 ) );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnSoundEmitterComponent::initialise( const Json::Value& Object )
{
	Super::initialise();
}

//////////////////////////////////////////////////////////////////////////
// play
void ScnSoundEmitterComponent::play( ScnSoundRef Sound )
{
	using namespace std::placeholders;

	if( SsCore::pImpl() )
	{
		// Acquire before playing (callback is threaded)
		getPackage()->acquire();

		// Get source from sound.
		SsSource* Source = Sound->getSource();

		// Play sample.
		SsChannel* Channel = SsCore::pImpl()->playSource( 
			Source, 
			Params_,
			std::bind( &ScnSoundEmitterComponent::onChannelDone, this, _1 ) );

		// Add to map, or release if not played.
		{
			std::lock_guard< std::recursive_mutex > Lock( ChannelSoundMutex_ );
			if( Channel != NULL )
			{
				ChannelSoundMap_[ Channel ] = Sound;
			}
			else
			{
				getPackage()->release();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// stopAll
void ScnSoundEmitterComponent::stopAll( BcBool ForceFlush )
{
	if( ForceFlush )
	{
		// Stop all channels. We are forcing a flush
		// so we know items will all be removed in the callback.
		ChannelSoundMutex_.lock();
		while( ChannelSoundMap_.size() > 0 )
		{
			auto It = ChannelSoundMap_.begin();
			ChannelSoundMutex_.unlock();
	
			// Stop channel.
			SsCore::pImpl()->stopChannel( (*It).first, BcTrue );
			ChannelSoundMutex_.lock();
		}
		ChannelSoundMutex_.unlock();
	}
	else
	{
		std::lock_guard< std::recursive_mutex > Lock( ChannelSoundMutex_ );

		// Stop channels.
		for( auto Pair : ChannelSoundMap_ )
		{
			SsCore::pImpl()->stopChannel( Pair.first );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// onChannelDone
void ScnSoundEmitterComponent::onChannelDone( SsChannel* Channel )
{
	std::lock_guard< std::recursive_mutex > Lock( ChannelSoundMutex_ );
	ChannelSoundMap_.erase( ChannelSoundMap_.find( Channel ) );
	getPackage()->release();
}

//////////////////////////////////////////////////////////////////////////
// setGain
void ScnSoundEmitterComponent::setGain( BcF32 Gain )
{
	Params_.Gain_ = Gain;
}

//////////////////////////////////////////////////////////////////////////
// setPitch
void ScnSoundEmitterComponent::setPitch( BcF32 Pitch )
{
	Params_.Pitch_ = Pitch;
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
	stopAll( BcTrue );

	Super::onDetach( Parent );
}
