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

#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"
#include "System/Sound/SsCore.h"

#include "System/SysKernel.h"

#include "Base/BcRandom.h"

#include "Base/BcMath.h"

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
		new ReField( "VelocityMultiplier_", &ScnSoundEmitterComponent::VelocityMultiplier_, bcRFF_IMPORTER ),
		new ReField( "VelocitySmoothingAmount_", &ScnSoundEmitterComponent::VelocitySmoothingAmount_, bcRFF_IMPORTER ),
		new ReField( "MaxVelocity_", &ScnSoundEmitterComponent::MaxVelocity_, bcRFF_IMPORTER ),

		new ReField( "LastPosition_", &ScnSoundEmitterComponent::LastPosition_, bcRFF_TRANSIENT ),
		new ReField( "Position_", &ScnSoundEmitterComponent::Position_, bcRFF_TRANSIENT ),
		new ReField( "Velocity_", &ScnSoundEmitterComponent::Velocity_, bcRFF_TRANSIENT ),
	};

	using namespace std::placeholders;
	ReRegisterClass< ScnSoundEmitterComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor( 
			{
				ScnComponentProcessFuncEntry(
					"Update",
					ScnComponentPriority::SOUND_EMITTER_UPDATE,
					std::bind( &ScnSoundEmitterComponent::update, _1 ) ),
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnSoundEmitterComponent::ScnSoundEmitterComponent():
	Gain_( 1.0f ),
	Pitch_( 1.0f ),
	MinDistance_( 1.0f ),
	MaxDistance_( 1000.0f ),
	AttenuationModel_( SsAttenuationModel::EXPONENTIAL ),
	RolloffFactor_( 0.01f ),
	VelocityMultiplier_( 1.0f ),
	VelocitySmoothingAmount_( 0.5f ),
	MaxVelocity_( 10.0f ),
	Position_( 0.0f, 0.0f, 0.0f ),
	Velocity_( 0.0f, 0.0f, 0.0f ),
	SmoothedVelocity_( 0.0f, 0.0f, 0.0f )
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
			Params.Velocity_ = SmoothedVelocity_;
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
// play
bool ScnSoundEmitterComponent::play( BcName PackageName, BcName SoundName, bool ContinuousUpdate )
{
	if( auto Package = CsCore::pImpl()->findPackage( PackageName ) )
	{
		ScnSoundRef Sound;
		if( CsCore::pImpl()->requestResource( PackageName, SoundName, Sound ) )
		{
			play( Sound, ContinuousUpdate );
			return true;
		}
	}

	return false;
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
// updateVelocity
void ScnSoundEmitterComponent::updateVelocity( BcF32 Tick )
{
	Velocity_ = ( ( LastPosition_ - Position_ ) / Tick ) * VelocityMultiplier_;
	VelocitySmoothingAmount_ = BcClamp( VelocitySmoothingAmount_, 0.0f, 1.0f );
	SmoothedVelocity_ = ( SmoothedVelocity_ * VelocitySmoothingAmount_ ) + ( Velocity_ * ( 1.0f - VelocitySmoothingAmount_ ) );
	if( SmoothedVelocity_.magnitude() > MaxVelocity_ )
	{
		SmoothedVelocity_ = SmoothedVelocity_.normal() * MaxVelocity_;
	}
}

//////////////////////////////////////////////////////////////////////////
// postUpdate
void ScnSoundEmitterComponent::updateEmitter( BcF32 Tick )
{
	BcAssert( SsCore::pImpl() );

	// Calculate velocity from change in position.
	LastPosition_ = Position_;
	Position_ = getParentEntity()->getWorldPosition();
	updateVelocity( Tick );

	// Setup parameters.
	// TODO: Per sound modifiers?
	SsChannelParams Params;
	Params.Gain_ = Gain_;
	Params.Pitch_ = Pitch_;
	Params.Min_ = MinDistance_;
	Params.Max_ = MaxDistance_;
	Params.AttenuationModel_ = AttenuationModel_;
	Params.RolloffFactor_ = RolloffFactor_;
	Params.Position_ = Position_;
	Params.Velocity_ = SmoothedVelocity_;
	
	// Update all channels required.
	std::lock_guard< std::recursive_mutex > Lock( ChannelSoundMutex_ );
	for( auto Channel : ChannelUpdateList_ )
	{
		SsCore::pImpl()->updateChannel( Channel, Params );
	}
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnSoundEmitterComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	LastPosition_ = Position_ = getParentEntity()->getWorldPosition();
}

//////////////////////////////////////////////////////////////////////////
// onDetach
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

//////////////////////////////////////////////////////////////////////////
// update
//static
void ScnSoundEmitterComponent::update( const ScnComponentList& Components )
{
	BcF32 Tick = SysKernel::pImpl()->getFrameTime();
	if( SsCore::pImpl() != nullptr )
	{
		for( auto Component : Components )
		{
			BcAssert( Component->isTypeOf< ScnSoundEmitterComponent >() );
			auto* EmitterComponent = static_cast< ScnSoundEmitterComponent* >( Component.get() );
			EmitterComponent->updateEmitter( Tick );
		}
	}
}