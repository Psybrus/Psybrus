/**************************************************************************
*
* File:		GaGameComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	29/12/11	
* Description:
*		Ball component.
*		
*
*
* 
**************************************************************************/

#include "GaGameComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaGameComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaGameComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	GameState_ = GS_INIT;
}

//////////////////////////////////////////////////////////////////////////
// GaGameComponent
//virtual
void GaGameComponent::update( BcReal Tick )
{
	switch( GameState_ )
	{
	case GS_INIT:
		{
			// Create entities and parent to ourself (oh god...will it work? FIRST TEST :3).
			ScnEntityRef ElementEntity = ScnCore::pImpl()->createEntity( "default", "In0ElementEntity" );

			// Attach to parent.
			getParentEntity()->attach( ElementEntity );

			// Switch game state.
			GameState_ = GS_UPDATE;
		}
		break;

	case GS_UPDATE:
		{
			// HUD stuff.
			BcMat4d Projection;
			Projection.orthoProjection( -640.0f, 640.0f, 360.0f, -360.0f, -1.0f, 1.0f );

			Canvas_->clear();
			Canvas_->pushMatrix( Projection );

			// Draw centred.
			BcVec2d Size = Font_->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), "TEST HUD", RsColour::WHITE, BcTrue );
			Font_->draw( Canvas_, BcVec2d( 0.0f, 0.0f ) - Size * 0.5f, "TEST HUD", RsColour::WHITE, BcFalse );

			Canvas_->popMatrix();
		}
		break;

	case GS_EXIT:
		{
		}
		break;
	}



	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// GaGameComponent
//virtual
void GaGameComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Find canvas component.
	for( BcU32 Idx = 0; Idx < Parent->getNoofComponents(); ++Idx )
	{
		ScnComponentRef Component( Parent->getComponent( Idx ) );

		if( Component->isTypeOf< ScnCanvasComponent >() )
		{
			Canvas_ = Component;
		}
		else if( Component->isTypeOf< ScnFontComponent >() )
		{
			Font_ = Component;
		}
	}

	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// GaGameComponent
//virtual
void GaGameComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Don't forget to detach!
	Super::onDetach( Parent );
}
