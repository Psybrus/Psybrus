/**************************************************************************
*
* File:		GaGameStateComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	15/12/12		
* Description:
*		Game state component. Logic.
*		
*
*
* 
**************************************************************************/

#include "GaGameStateComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaGameStateComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaGameStateComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );
}

//////////////////////////////////////////////////////////////////////////
// GaGameStateComponent
//virtual
void GaGameStateComponent::update( BcReal Tick )
{
	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// GaGameStateComponent
//virtual
void GaGameStateComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Spawn level.
	ScnEntityRef LevelEntity = ScnCore::pImpl()->createEntity( "default", "LevelEntity" );
	Parent->attach( LevelEntity );

	// Spawn player.
	ScnEntityRef PlayerEntity = ScnCore::pImpl()->createEntity( "default", "PlayerEntity" );
	Parent->attach( PlayerEntity );

	// Spawn enemy.
	ScnEntityRef PlayerEntity = ScnCore::pImpl()->createEntity( "default", "EnemyEntity" );
	Parent->attach( PlayerEntity );

	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// GaGameStateComponent
//virtual
void GaGameStateComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Don't forget to detach!
	Super::onDetach( Parent );
}

