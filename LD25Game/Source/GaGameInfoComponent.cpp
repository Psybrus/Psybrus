/**************************************************************************
*
* File:		GaGameInfoComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	15/12/12		
* Description:
*		Game info component. Config and stuff.
*		
*
*
* 
**************************************************************************/

#include "GaGameInfoComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaGameInfoComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaGameInfoComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );
}

//////////////////////////////////////////////////////////////////////////
// GaGameInfoComponent
//virtual
void GaGameInfoComponent::update( BcReal Tick )
{
	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// GaGameInfoComponent
//virtual
void GaGameInfoComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// GaGameInfoComponent
//virtual
void GaGameInfoComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Don't forget to detach!
	Super::onDetach( Parent );
}

