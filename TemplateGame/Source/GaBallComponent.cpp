/**************************************************************************
*
* File:		GaBallComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	29/12/11	
* Description:
*		Ball component.
*		
*
*
* 
**************************************************************************/

#include "GaBallComponent.h"

#include "System/Audiokinetic/AkEvents.h"
#include "GeneratedSoundBanks/Wwise_IDs.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaBallComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaBallComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );
	
	Rotation_ = 0.0f;
	Velocity_ = BcVec3d( 8.0f, 0.0f, 8.0f );
}

//////////////////////////////////////////////////////////////////////////
// GaBallComponent
//virtual
void GaBallComponent::update( BcReal Tick )
{
	// Setup rotation.
	BcQuat Rotation;
	Rotation.fromEular( Rotation_, 0.0f, 0.0f );
	Rotation_ += Tick;
	//getParentEntity()->setRotation( Rotation );

	// Move ball.
	BcBool HasBounced = BcFalse;
	const BcVec3d& Position = getParentEntity()->getMatrix().translation();
	BcVec3d NewPosition = Position + Velocity_ * Tick;

	if( NewPosition.x() < -18.0f || NewPosition.x() > 18.0f )
	{
		NewPosition -= BcVec3d( Velocity_.x(), 0.0f, 0.0f ) * Tick;
		Velocity_.x( -Velocity_.x() );
		HasBounced = BcTrue;
	}
	
	if( NewPosition.z() < -10.0f || NewPosition.z() > 10.0f )
	{
		NewPosition -= BcVec3d( 0.0f, 0.0f, Velocity_.z() ) * Tick;
		Velocity_.z( -Velocity_.z() );
		HasBounced = BcTrue;
	}

	if( HasBounced )
	{
		getParentEntity()->publish( akEVT_CORE_POST, AkEventPost( AK::EVENTS::RABBITCHEW_PLAY ), BcFalse );
	}

	getParentEntity()->setPosition( NewPosition );
}

//////////////////////////////////////////////////////////////////////////
// GaBallComponent
//virtual
void GaBallComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Grab the paddles, dirty way but it works.


	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// GaBallComponent
//virtual
void GaBallComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Don't forget to detach!
	Super::onDetach( Parent );
}

