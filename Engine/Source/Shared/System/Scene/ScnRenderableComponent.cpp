/**************************************************************************
*
* File:		ScnRenderableComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	28/12/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnRenderableComponent.h"
#include "RsCore.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnRenderableComponent );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnRenderableComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "ScnRenderableComponent" )
	.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
//virtual
void ScnRenderableComponent::update( BcReal Tick )
{
	Super::update( Tick );

}

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
//virtual
void ScnRenderableComponent::render( RsFrame* pFrame, RsRenderSort Sort )
{
	// Do nothing.
}

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
//virtual
void ScnRenderableComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	// NEILO TODO: Add to scene.
}

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
//virtual
void ScnRenderableComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	// NEILO TODO: Add to scene.
}
