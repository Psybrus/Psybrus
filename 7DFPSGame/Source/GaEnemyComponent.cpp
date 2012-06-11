/**************************************************************************
*
* File:		GaEnemyComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		Enemy component.
*		
*
*
* 
**************************************************************************/

#include "GaEnemyComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaEnemyComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaEnemyComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );
}

//////////////////////////////////////////////////////////////////////////
// GaEnemyComponent
//virtual
void GaEnemyComponent::update( BcReal Tick )
{
	// Find move vector.
	BcReal MoveSpeed = 2.0f;
	BcVec3d MoveVector;

	//
	
	// Set the move.
	BcVec3d AppliedMoveVector = MoveVector;
	AppliedMoveVector.z( 0.0f );
	AppliedMoveVector = AppliedMoveVector.normal() * MoveSpeed;
	Pawn_->setMove( AppliedMoveVector );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaEnemyComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Don't forget to attach!
	Super::onAttach( Parent );

	// Cache world stuff we need.
	ScnEntityRef WorldEntity = ScnCore::pImpl()->findEntity( "WorldEntity_0" );
	Canvas_ = WorldEntity->getComponentByType< ScnCanvasComponent >( 0 );
	BSP_ = WorldEntity->getComponentByType< GaWorldBSPComponent >( 0 );
	Pressure_ = WorldEntity->getComponentByType< GaWorldPressureComponent >( 0 );

	// Grab pawn.
	Pawn_ = Parent->getComponentByType< GaPawnComponent >( 0 );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaEnemyComponent::onDetach( ScnEntityWeakRef Parent )
{
	Canvas_ = NULL;
	BSP_ = NULL;
	Pressure_ = NULL;
	Pawn_ = NULL;

	// Don't forget to detach!
	Super::onDetach( Parent );
}
