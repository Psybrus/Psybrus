/**************************************************************************
*
* File:		GaEnemyComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	15/12/12		
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
	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// GaEnemyComponent
//virtual
void GaEnemyComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// GaEnemyComponent
//virtual
void GaEnemyComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Don't forget to detach!
	Super::onDetach( Parent );
}

