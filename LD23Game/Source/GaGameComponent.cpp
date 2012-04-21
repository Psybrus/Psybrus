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
// update
//virtual
void GaGameComponent::update( BcReal Tick )
{
	switch( GameState_ )
	{
	case GS_INIT:
		{
			// Spawn a bunch of entities for the level.
			for( BcU32 Idx = 0; Idx < 8; ++Idx )
			{
				BcVec3d Position0 = BcRandom::Global.randVec3Normal() * 20.0f; 
				BcVec3d Position1 = BcRandom::Global.randVec3Normal() * 20.0f; 
				spawnElement( Position0, "In0ElementEntity" );
				spawnElement( Position1, "In1ElementEntity" );
			}
			
			// Switch game state.
			GameState_ = GS_UPDATE;
		}
		break;

	case GS_UPDATE:
		{
			// Update.
			updateSimulation( Tick );


			// HUD stuff.
			BcMat4d Projection;
			Projection.orthoProjection( -640.0f, 640.0f, 360.0f, -360.0f, -1.0f, 1.0f );

			Canvas_->clear();
			Canvas_->pushMatrix( Projection );

			// Draw centred.
			BcVec2d Size = Font_->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), "TEST HUD", RsColour::WHITE, BcTrue );
			Font_->draw( Canvas_, BcVec2d( -400.0f, -300.0f ) - Size * 0.5f, "TEST HUD", RsColour::WHITE, BcFalse );

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
// onAttach
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
// onDetach
//virtual
void GaGameComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Don't forget to detach!
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// spawnElement
void GaGameComponent::spawnElement( const BcVec3d& Position, const BcName& Type )
{
	ScnEntityRef Entity( ScnCore::pImpl()->createEntity( "default", Type ) );
	BcAssertMsg( Entity.isValid(), "Can't spawn element." );

	GaElementComponentRef Element;
	for( BcU32 Idx = 0; Idx < Entity->getNoofComponents(); ++Idx )
	{
		ScnComponentRef Component( Entity->getComponent( Idx ) );

		if( Component->isTypeOf< GaElementComponent >() )
		{
			Element = Component;
			break;
		}
	}

	BcAssertMsg( Element.isValid(), "Element not in entity. Did you use the right template?" );

	// Calculate velocity.
	BcVec3d Velocity = ( -Position ).normal() * Element->Direction_;

	// Add a little randomisation.
	Velocity += ( BcRandom::Global.randVec3Normal() * 0.3f );
	
	// Clamp, and TODO: make speeds vary a little.
	Velocity = Velocity.normal() * Element->MaxSpeed_;
	
	TElement ElementInternal = 
	{
		Type,
		Entity,
		Element,
		Position,
		Velocity,
	};

	ElementList_.push_back( ElementInternal );

	// Attach the entity to our parent entity for sequential logic.
	getParentEntity()->attach( Entity );
}

//////////////////////////////////////////////////////////////////////////
// updateSimulation
void GaGameComponent::updateSimulation( BcReal Tick )
{
	for( BcU32 Idx = 0; Idx < ElementList_.size(); ++Idx )
	{
		TElement& Element( ElementList_[ Idx ] );

		// Find nearest that isn't us, and repel based on distance.
		BcU32 OtherIdx = findNearestOfType( Element.Position_, BcName::INVALID, Idx );
		if( OtherIdx != NULL )
		{
			TElement& OtherElement( ElementList_[ OtherIdx ] );
			
			
		}
		
		// Advance.
		Element.Position_ += Element.Velocity_ * Tick;

		// Set entity position.
		Element.Entity_->setPosition( Element.Position_ );
	}
}

//////////////////////////////////////////////////////////////////////////
// findNearestOfType
BcU32 GaGameComponent::findNearestOfType( const BcVec3d& Position, const BcName& Type, BcU32 Exclude )
{
	BcU32 Nearest = BcErrorCode;
	BcReal NearestDistanceSquared = 1e24f;
	for( BcU32 Idx = 0; Idx < ElementList_.size(); ++Idx )
	{
		TElement& Element( ElementList_[ Idx ] );

		if( Exclude != Idx && ( Type == BcName::INVALID || Element.Type_ == Type ) )
		{
			BcReal DistanceSquared = ( Position - Element.Position_ ).magnitudeSquared();

			if( DistanceSquared < NearestDistanceSquared )
			{
				Nearest = Idx;
				NearestDistanceSquared = DistanceSquared;
			}
		}	
	}

	return Nearest;
}