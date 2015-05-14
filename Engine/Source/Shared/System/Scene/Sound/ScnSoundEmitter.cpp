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
REFLECTION_DEFINE_DERIVED( ScnSoundEmitterComponent );

void ScnSoundEmitterComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Gain_", &ScnSoundEmitterComponent::Gain_, bcRFF_IMPORTER ),
		new ReField( "Pitch_", &ScnSoundEmitterComponent::Pitch_, bcRFF_IMPORTER ),
		new ReField( "MinDistance_", &ScnSoundEmitterComponent::MinDistance_, bcRFF_IMPORTER ),
		new ReField( "MaxDistance_", &ScnSoundEmitterComponent::MaxDistance_, bcRFF_IMPORTER ),
		new ReField( "AttenuationModel_", &ScnSoundEmitterComponent::AttenuationModel_, bcRFF_IMPORTER ),
		new ReField( "RolloffFactor_", &ScnSoundEmitterComponent::RolloffFactor_, bcRFF_IMPORTER ),

		new ReField( "LastPosition_", &ScnSoundEmitterComponent::LastPosition_, bcRFF_TRANSIENT ),
		new ReField( "Position_", &ScnSoundEmitterComponent::Position_, bcRFF_TRANSIENT ),
		new ReField( "Velocity_", &ScnSoundEmitterComponent::Velocity_, bcRFF_TRANSIENT ),
	};
		
	ReRegisterClass< ScnSoundEmitterComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( -2010 ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnSoundEmitterComponent::ScnSoundEmitterComponent():
	Gain_( 1.0f ),
	Pitch_( 1.0f ),
	MinDistance_( 1.0f ),
	MaxDistance_( 1000.0f ),
	AttenuationModel_( SsAttenuationModel::EXPONENTIAL ),
	RolloffFactor_( 1.0f ),
	Position_( MaVec3d( 0.0f, 0.0f, 0.0f ) ),
	Velocity_( MaVec3d( 0.0f, 0.0f, 0.0f ) )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnSoundEmitterComponent::~ScnSoundEmitterComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// play
void ScnSoundEmitterComponent::play( ScnSoundRef Sound, bool ContinuousUpdate )
{
	using namespace std::placeholders;

	if( SsCore::pImpl() )
	{
		// Acquire before playing (callback is threaded)
		getPackage()->acquire();
		Sound->getPackage()->acquire();

		// Get source from sound.
		SsSource* Source = Sound->getSource();
		BcAssert( Source );

		// Setup channel params for play.
		SsChannelParams Params;
		Params.Gain_ = Gain_;
		Params.Pitch_ = Pitch_;
		Params.Min_ = MinDistance_;
		Params.Max_ = MaxDistance_;
		Params.AttenuationModel_ = AttenuationModel_;
		Params.RolloffFactor_ = RolloffFactor_;
		Params.Position_ = Position_;
		// Only continuous update should have velocity.
		if( ContinuousUpdate )
		{
			Params.Velocity_ = Velocity_;
		}
		else
		{
			Params.Velocity_ = MaVec3d( 0.0f, 0.0f, 0.0f );
		}


		// Play sample.
		SsChannel* Channel = SsCore::pImpl()->playSource( 
			Source, 
			Params,
			std::bind( &ScnSoundEmitterComponent::onChannelDone, this, _1 ) );

		// Add to map, or release if not played.
		{
			std::lock_guard< std::recursive_mutex > Lock( ChannelSoundMutex_ );
			if( Channel != nullptr )
			{
				ChannelSoundMap_[ Channel ] = Sound;
				if( ContinuousUpdate )
				{
					ChannelUpdateList_.push_back( Channel );
				}
			}
			else
			{
				getPackage()->release();
				Sound->getPackage()->release();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// stopAll
void ScnSoundEmitterComponent::stopAll( bool ForceFlush )
{
	if( SsCore::pImpl() )
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
}

//////////////////////////////////////////////////////////////////////////
// setGain
void ScnSoundEmitterComponent::setGain( BcF32 Gain )
{
	BcAssert( Gain >=- 0.0f );
	Gain_ = Gain;
}

//////////////////////////////////////////////////////////////////////////
// setPitch
void ScnSoundEmitterComponent::setPitch( BcF32 Pitch )
{
	BcAssert( Pitch > 0.0f );
	Pitch_ = Pitch;
}

//////////////////////////////////////////////////////////////////////////
// setMinMaxDistance
void ScnSoundEmitterComponent::setMinMaxDistance( BcF32 MinDistance, BcF32 MaxDistance )
{
	BcAssert( MinDistance > 0.0f );
	BcAssert( MaxDistance > MinDistance );
	MinDistance_ = MinDistance;
	MaxDistance_ = MaxDistance;
}

//////////////////////////////////////////////////////////////////////////
// setAttenuation
void ScnSoundEmitterComponent::setAttenuation( SsAttenuationModel AttenuationModel, BcF32 RolloffFactor )
{
	BcAssert( RolloffFactor > 0.0f );
	AttenuationModel_ = AttenuationModel;
	RolloffFactor_ = RolloffFactor;
}

//////////////////////////////////////////////////////////////////////////
// update
void ScnSoundEmitterComponent::update( BcF32 Tick )
{
	if( SsCore::pImpl() )
	{
		// Calculate velocity from change in position.
		LastPosition_ = Position_;
		Position_ = getParentEntity()->getWorldPosition();
		Velocity_ = ( Position_ - LastPosition_ ) / Tick;

		// Setup parameters.		
		SsChannelParams Params;
		Params.Gain_ = Gain_;
		Params.Pitch_ = Pitch_;
		Params.Min_ = MinDistance_;
		Params.Max_ = MaxDistance_;
		Params.AttenuationModel_ = AttenuationModel_;
		Params.RolloffFactor_ = RolloffFactor_;
		Params.Position_ = Position_;
		Params.Velocity_ = Velocity_;

		// Update all channels required.
		std::lock_guard< std::recursive_mutex > Lock( ChannelSoundMutex_ );
		for( auto Channel : ChannelUpdateList_ )
		{
			SsCore::pImpl()->updateChannel( Channel, Params );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setPitch
//virtual
void ScnSoundEmitterComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	LastPosition_ = Position_ = getParentEntity()->getWorldPosition();
}

//////////////////////////////////////////////////////////////////////////
// setPitch
//virtual
void ScnSoundEmitterComponent::onDetach( ScnEntityWeakRef Parent )
{
	stopAll( BcTrue );

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onChannelDone
void ScnSoundEmitterComponent::onChannelDone( SsChannel* Channel )
{
	std::lock_guard< std::recursive_mutex > Lock( ChannelSoundMutex_ );
	auto FoundIt = ChannelSoundMap_.find( Channel );

	// Release packages now that we're done with resources.
	getPackage()->release();
	FoundIt->second->getPackage()->release();

	// Remove.
	ChannelSoundMap_.erase( FoundIt );
	std::remove( ChannelUpdateList_.begin(), ChannelUpdateList_.end(), Channel );
}
