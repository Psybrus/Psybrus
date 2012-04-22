/**************************************************************************
*
* File:		GaCameraComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	21/04/12
* Description:
*		Example user component.
*		
*
*
* 
**************************************************************************/

#include "GaCameraComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaCameraComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaCameraComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	Ticker_ = 0.0f;
	TargetPosition_ = BcVec3d( 0.0f, 60.0f, -2.0f );
	Position_ = BcVec3d( 0.0f, 400.0f, -2.0f );
}

//////////////////////////////////////////////////////////////////////////
// GaCameraComponent
//virtual
void GaCameraComponent::update( BcReal Tick )
{
	Ticker_ += Tick * 1.0f;

	//BcVec3d Position( BcVec3d( BcCos( Ticker_ ) , 1.0f, -BcSin( Ticker_ ) ) * 50.0f );

	Position_ = Position_ * 0.99f + TargetPosition_ * 0.01f;
	BcVec3d OffsetPosition( BcCos( Ticker_ * 0.3f ), BcCos( Ticker_ * 0.1f ), -BcSin( Ticker_ * 0.5f ) );
	
	// Setup entity position to render from.
	BcMat4d LookAt;
	LookAt.lookAt( Position_ + OffsetPosition, BcVec3d( 0.0f, 0.0f, 0.0f ), BcVec3d( 0.0f, .0f, 1.0f ) );
	LookAt.inverse();
	getParentEntity()->setMatrix( LookAt );
}

//////////////////////////////////////////////////////////////////////////
// GaCameraComponent
//virtual
void GaCameraComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// GaCameraComponent
//virtual
void GaCameraComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Don't forget to detach!
	Super::onDetach( Parent );
}

