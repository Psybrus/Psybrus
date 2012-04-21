/**************************************************************************
*
* File:		GaElementComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	29/12/11	
* Description:
*		Ball component.
*		
*
*
* 
**************************************************************************/

#include "GaElementComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaElementComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaElementComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );
}

//////////////////////////////////////////////////////////////////////////
// GaElementComponent
//virtual
void GaElementComponent::update( BcReal Tick )
{
	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// GaElementComponent
//virtual
void GaElementComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// GaElementComponent
//virtual
void GaElementComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Don't forget to detach!
	Super::onDetach( Parent );
}

