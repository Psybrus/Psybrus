/**************************************************************************
*
* File:		GaPaddleComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	29/12/11	
* Description:
*		Example user component.
*		
*
*
* 
**************************************************************************/

#include "GaPaddleComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaPaddleComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaPaddleComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );
}

//////////////////////////////////////////////////////////////////////////
// GaPaddleComponent
//virtual
void GaPaddleComponent::update( BcReal Tick )
{

}

//////////////////////////////////////////////////////////////////////////
// GaPaddleComponent
//virtual
void GaPaddleComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// GaPaddleComponent
//virtual
void GaPaddleComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Don't forget to detach!
	Super::onDetach( Parent );
}

