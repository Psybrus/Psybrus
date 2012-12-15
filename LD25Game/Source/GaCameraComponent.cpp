/**************************************************************************
*
* File:		GaCameraComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	15/12/12	
* Description:
*		Camera component.
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
}

//////////////////////////////////////////////////////////////////////////
// GaCameraComponent
//virtual
void GaCameraComponent::update( BcReal Tick )
{
	Super::update( Tick );

	BcMat4d Matrix;
	Matrix.rotation( BcVec3d( BcPIDIV2 - ( BcPI / 16.0f ), 0.0f, 0.0f ) );
	Matrix.translation( BcVec3d( 0.0f, -2.0f, -1.0f ) );
	getParentEntity()->setMatrix( Matrix );
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

