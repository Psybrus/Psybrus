/**************************************************************************
*
* File:		GaProjectileComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	16/12/2012
* Description:
*		Projectile component.
*		
*
*
* 
**************************************************************************/

#include "GaProjectileComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaProjectileComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaProjectileComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	AngularVelocity_ = BcRandom::Global.randVec3() * 2.0f;
}

//////////////////////////////////////////////////////////////////////////
// setPositionVelocity
void GaProjectileComponent::setPositionVelocity( const BcVec3d& Position, const BcVec3d& Velocity )
{
	Position_ = Position;
	Velocity_ = Velocity;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaProjectileComponent::update( BcReal Tick )
{
	Position_ += Velocity_ * Tick;
	Rotation_ += AngularVelocity_ * Tick;

	BcMat4d Matrix;
	Matrix.rotation( Rotation_ );
	Matrix.translation( Position_ );
	getParentEntity()->setMatrix( Matrix );

	GaEventPosition Event;
	Event.pSender_ = this;
	Event.Position_ = Position_;
	getParentEntity()->getParentEntity()->publish( gaEVT_PROJECTILE_POSITION, Event );

	if( Position_.y() < -1.0f )
	{
		getParentEntity()->detachFromParent();
	}
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaProjectileComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Don't forget to attach!
	Super::onAttach( Parent );


}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaProjectileComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Don't forget to detach!
	Super::onDetach( Parent );
}
