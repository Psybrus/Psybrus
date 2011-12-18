/**************************************************************************
*
* File:		GaPlayerEntity.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Player.
*		
*
*
* 
**************************************************************************/

#include "GaPlayerEntity.h"

#include "GaTopState.h"
#include "GaMainGameState.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
GaPlayerEntity::GaPlayerEntity():
	pBody_( new GaPhysicsBody( BcVec2d( -256.0f, 0.0f ), 128.0f, 1024.0f ) )
{
	TargetPosition_ = pBody_->update( 0.0f );

	// Bind input events.
	OsEventInputMouse::Delegate OnMouseMove = OsEventInputMouse::Delegate::bind< GaPlayerEntity, &GaPlayerEntity::onMouseMove >( this );
	OsEventInputMouse::Delegate OnMouseDown = OsEventInputMouse::Delegate::bind< GaPlayerEntity, &GaPlayerEntity::onMouseDown >( this );
	OsEventInputMouse::Delegate OnMouseUp = OsEventInputMouse::Delegate::bind< GaPlayerEntity, &GaPlayerEntity::onMouseUp >( this );

	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, OnMouseMove );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseDown );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEUP, OnMouseUp );

	ScnMaterialRef Material;
	GaTopState::pImpl()->getMaterial( GaTopState::MATERIAL_KITTY, Material );
	BunnyRenderer_.setMaterial( Material, BcVec3d( 0.6f, 0.6f, 0.6f ) );

}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaPlayerEntity::~GaPlayerEntity()
{
	// Unbind all events.
	OsCore::pImpl()->unsubscribeAll( this );
}

////////////////////////////////////////////////////////////////////////////////
// update
void GaPlayerEntity::update( BcReal Tick )
{
	// Tell the body to target a position, and wander around a little.
	pBody_->reset();
	pBody_->target( TargetPosition_, 256.0f );
	pBody_->wander( 8.0f );

	// Update body.
	Position_ = pBody_->update( Tick );

	BunnyRenderer_.update( Tick );
}

////////////////////////////////////////////////////////////////////////////////
// render
void GaPlayerEntity::render( ScnCanvasRef Canvas )
{
	BunnyRenderer_.render( pParent(), Canvas, BcVec3d( Position_.x(), Position_.y(), 0.0f ), pBody_->Velocity_ );
}

////////////////////////////////////////////////////////////////////////////////
// onMouseMove
eEvtReturn GaPlayerEntity::onMouseMove( EvtID ID, const OsEventInputMouse& Event )
{
	if( Event.ButtonCode_ == 0 )
	{
		BcVec3d Near, Far;
		pParent()->getWorldPosition( BcVec2d( Event.MouseX_, Event.MouseY_ ), Near, Far );
		
		BcPlane GroundPlane( BcVec3d( 0.0f, 0.0f, 1.0f ), 0.0f );

		BcReal Distance = 0.0f;

		BcVec3d Intersection; 
		if( GroundPlane.lineIntersection( Near, Far, Intersection ) )
		{
			TargetPosition_ = BcVec2d( Intersection.x(), Intersection.y() );
		}
		
	}
	return evtRET_PASS;
}

////////////////////////////////////////////////////////////////////////////////
// onMouseDown
eEvtReturn GaPlayerEntity::onMouseDown( EvtID ID, const OsEventInputMouse& Event )
{
	return evtRET_PASS;
}

////////////////////////////////////////////////////////////////////////////////
// onMouseUp
eEvtReturn GaPlayerEntity::onMouseUp( EvtID ID, const OsEventInputMouse& Event )
{
	return evtRET_PASS;
}
