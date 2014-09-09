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

#ifdef PSY_SERVER
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
		new ReField( "Position_",			&ScnSoundEmitterComponent::Position_ ),
		new ReField( "Gain_",				&ScnSoundEmitterComponent::Gain_ ),
		new ReField( "Pitch_",				&ScnSoundEmitterComponent::Pitch_ ),
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

	Position_ = MaVec3d( 0.0f, 0.0f, 0.0f );
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
	Super::destroy();
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
			std::lock_guard< std::mutex > Lock( ChannelSoundMutex_ );
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
void ScnSoundEmitterComponent::stopAll()
{
	std::lock_guard< std::mutex > Lock( ChannelSoundMutex_ );

	// Stop all bound channels.
	for( TChannelSoundMapIterator It( ChannelSoundMap_.begin() ); It != ChannelSoundMap_.end(); ++It )
	{
		// Stop channel.
		//SsCore::pImpl()->stopChannel( (*It).first );
	}
}

//////////////////////////////////////////////////////////////////////////
// onChannelDone
void ScnSoundEmitterComponent::onChannelDone( SsChannel* Channel )
{
	std::lock_guard< std::mutex > Lock( ChannelSoundMutex_ );
	ChannelSoundMap_.erase( ChannelSoundMap_.find( Channel ) );
	getPackage()->release();
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
