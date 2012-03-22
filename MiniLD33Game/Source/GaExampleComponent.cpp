/**************************************************************************
*
* File:		GaExampleComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	29/12/11	
* Description:
*		Example user component.
*		
*
*
* 
**************************************************************************/

#include "GaExampleComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaExampleComponent );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void GaExampleComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "GaExampleComponent" )
	.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaExampleComponent::initialise()
{
	Super::initialise();

	Rotation_ = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// GaExampleComponent
//virtual
void GaExampleComponent::update( BcReal Tick )
{
	// Setup rotation.
	BcQuat Rotation;
	Rotation.fromEular( Rotation_, 0.0f, 0.0f );
	Rotation_ += Tick;
	getParentEntity()->setRotation( Rotation );

	if( CanvasComponent_.isValid() )
	{
		CanvasComponent_->drawLine( BcVec2d( -1.0f, -1.0f ), BcVec2d( 1.0f, 1.0f ), RsColour::GREEN, 0 );
	}
}

//////////////////////////////////////////////////////////////////////////
// GaExampleComponent
//virtual
void GaExampleComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Find canvas component on parent. TODO: Make a utility function for this.
	for( BcU32 Idx = 0; Idx < Parent->getNoofComponents(); ++Idx )
	{
		CanvasComponent_ = Parent->getComponent( Idx );

		if( CanvasComponent_.isValid() )
		{
			break;
		}
	}

	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// GaExampleComponent
//virtual
void GaExampleComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Null canvas reference.
	CanvasComponent_ = NULL;

	// Don't forget to detach!
	Super::onDetach( Parent );
}

