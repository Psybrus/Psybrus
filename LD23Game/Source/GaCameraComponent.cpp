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
}

//////////////////////////////////////////////////////////////////////////
// GaCameraComponent
//virtual
void GaCameraComponent::update( BcReal Tick )
{
	Ticker_ += Tick * 1.0f;

	//BcVec3d Position( BcVec3d( BcCos( Ticker_ ) , 1.0f, -BcSin( Ticker_ ) ) * 50.0f );

	BcVec3d Position( 0.0f, 40.0f, -1.0f );

	// Setup entity position to render from.
	BcMat4d LookAt;
	LookAt.lookAt( Position, BcVec3d( 0.0f, 0.0f, 0.0f ), BcVec3d( 0.0f, .0f, 1.0f ) );
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

