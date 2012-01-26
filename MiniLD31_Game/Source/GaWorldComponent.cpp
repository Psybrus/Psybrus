/**************************************************************************
*
* File:		GaWorldComponent.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		World.
*		
*
*
* 
**************************************************************************/

#include "GaWorldComponent.h"

#include "GaTopState.h"

////////////////////////////////////////////////////////////////////////////////
// Define resource.
DEFINE_RESOURCE( GaWorldComponent )

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void GaWorldComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "GaWorldComponent" )
	.endCatagory();
}

////////////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void GaWorldComponent::initialise()
{
	GeneratedNodeGraph_ = BcFalse;	
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaWorldComponent::update( BcReal Tick )
{
	if( GeneratedNodeGraph_ == BcFalse )
	{


		GeneratedNodeGraph_ = BcTrue;
	}
}

////////////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaWorldComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );	

	for( BcU32 Idx = 0; Idx < getParentEntity()->getNoofComponents(); ++Idx )
	{
		ScnCanvasComponentRef CanvasComponent( getParentEntity()->getComponent( Idx ) );
		if( CanvasComponent.isValid() )
		{
			CanvasComponent_ = CanvasComponent;
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaWorldComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}
