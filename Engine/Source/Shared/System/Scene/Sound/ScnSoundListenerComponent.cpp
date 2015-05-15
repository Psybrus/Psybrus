/**************************************************************************
*
* File:		ScnSoundListenerComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	23/04/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Sound/ScnSoundListenerComponent.h"

#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"
#include "System/Sound/SsCore.h"

#include "Base/BcMath.h"

#ifdef PSY_IMPORT_PIPELINE
#include "Base/BcFile.h"
#include "Base/BcStream.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnSoundListenerComponent );

void ScnSoundListenerComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "VelocityMultiplier_", &ScnSoundListenerComponent::VelocityMultiplier_, bcRFF_IMPORTER ),
		new ReField( "VelocitySmoothingAmount_", &ScnSoundListenerComponent::VelocitySmoothingAmount_, bcRFF_IMPORTER ),
		new ReField( "MaxVelocity_", &ScnSoundListenerComponent::MaxVelocity_, bcRFF_IMPORTER ),

		new ReField( "LastPosition_", &ScnSoundListenerComponent::LastPosition_, bcRFF_TRANSIENT ),
		new ReField( "Position_", &ScnSoundListenerComponent::Position_, bcRFF_TRANSIENT ),
		new ReField( "Velocity_", &ScnSoundListenerComponent::Velocity_, bcRFF_TRANSIENT ),
		new ReField( "SmoothedVelocity_", &ScnSoundListenerComponent::SmoothedVelocity_, bcRFF_TRANSIENT ),
	};

	ReRegisterClass< ScnSoundListenerComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( -2020 ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnSoundListenerComponent::ScnSoundListenerComponent():
	VelocityMultiplier_( 1.0f ),
	VelocitySmoothingAmount_( 0.5f ),
	MaxVelocity_( 10.0f ),
	LastPosition_( 0.0f, 0.0f, 0.0f ),
	Position_( 0.0f, 0.0f, 0.0f ),
	Velocity_( 0.0f, 0.0f, 0.0f ),
	SmoothedVelocity_( 0.0f, 0.0f, 0.0f )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnSoundListenerComponent::~ScnSoundListenerComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnSoundListenerComponent::postUpdate( BcF32 Tick )
{
	Super::postUpdate( Tick );

	if( SsCore::pImpl() != nullptr )
	{
		LastPosition_ = Position_;
		Position_ = getParentEntity()->getWorldPosition();
		Velocity_ = ( ( LastPosition_ - Position_ ) / Tick ) * VelocityMultiplier_;

		// TODO: Smooth this be smoothing based on tick.
		VelocitySmoothingAmount_ = BcClamp( VelocitySmoothingAmount_, 0.0f, 1.0f );
		SmoothedVelocity_ = ( SmoothedVelocity_ * VelocitySmoothingAmount_ ) + ( Velocity_ * ( 1.0f - VelocitySmoothingAmount_ ) );
		if( SmoothedVelocity_.magnitude() > MaxVelocity_ )
		{
			SmoothedVelocity_ = SmoothedVelocity_.normal() * MaxVelocity_;
		}

		MaMat4d Matrix = getParentEntity()->getWorldMatrix();
		SsCore::pImpl()->setListener( Matrix, SmoothedVelocity_ );
	}
}

//////////////////////////////////////////////////////////////////////////
// setPitch
//virtual
void ScnSoundListenerComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	LastPosition_ = Position_ = getParentEntity()->getWorldPosition();;
}

//////////////////////////////////////////////////////////////////////////
// setPitch
//virtual
void ScnSoundListenerComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}
