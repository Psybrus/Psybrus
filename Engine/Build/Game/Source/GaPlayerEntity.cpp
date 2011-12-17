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

////////////////////////////////////////////////////////////////////////////////
// Ctor
GaPlayerEntity::GaPlayerEntity( const BcMat4d& Projection ):
	Body_( BcVec2d( 0.0f, 0.0f ), 128.0f, 256.0f )
{
	//
	Projection_ = Projection;
	Position_ = BcVec2d( 0.0f, 0.0f );
	TargetPosition_ = BcVec2d( 0.0f, 0.0f );

	// Bind input events.
	OsEventInputMouse::Delegate OnMouseMove = OsEventInputMouse::Delegate::bind< GaPlayerEntity, &GaPlayerEntity::onMouseMove >( this );
	OsEventInputMouse::Delegate OnMouseDown = OsEventInputMouse::Delegate::bind< GaPlayerEntity, &GaPlayerEntity::onMouseDown >( this );
	OsEventInputMouse::Delegate OnMouseUp = OsEventInputMouse::Delegate::bind< GaPlayerEntity, &GaPlayerEntity::onMouseUp >( this );

	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, OnMouseMove );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseDown );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEUP, OnMouseUp );
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
	// Tell body to target position.
	Body_.target( TargetPosition_, 256.0f );

	// Update body.
	Position_ = Body_.update( Tick );
}

////////////////////////////////////////////////////////////////////////////////
// render
void GaPlayerEntity::render( ScnCanvasRef Canvas )
{
	BcVec2d HalfSize( 16.0f, 16.0f );
	BcVec2d QuarterSize( 8.0f, 8.0f );
	Canvas->drawBox( Position_ - HalfSize, Position_ + HalfSize, RsColour::GREEN, 0 );

	// DEBUG DATA.
	Canvas->drawLine( Position_, Position_ + Body_.Velocity_, RsColour::WHITE, 1 );
	Canvas->drawLine( Position_, Position_ + Body_.Acceleration_, RsColour::RED, 1 );
	Canvas->drawBox( TargetPosition_ - QuarterSize, TargetPosition_ + QuarterSize, RsColour::RED, 0 );
}

////////////////////////////////////////////////////////////////////////////////
// onMouseMove
eEvtReturn GaPlayerEntity::onMouseMove( EvtID ID, const OsEventInputMouse& Event )
{
	return evtRET_PASS;
}

////////////////////////////////////////////////////////////////////////////////
// onMouseDown
eEvtReturn GaPlayerEntity::onMouseDown( EvtID ID, const OsEventInputMouse& Event )
{
	BcVec2d HalfResolution = BcVec2d( GResolutionWidth / 2, GResolutionHeight / 2 );
	BcVec2d CursorPosition = BcVec2d( Event.MouseX_, GResolutionHeight - Event.MouseY_ );
	BcVec2d ScreenPosition = ( CursorPosition - HalfResolution ) / HalfResolution;

	BcMat4d InverseProjection( Projection_ );
	InverseProjection.inverse();

	TargetPosition_ = ScreenPosition * InverseProjection;

	return evtRET_PASS;
}

////////////////////////////////////////////////////////////////////////////////
// onMouseUp
eEvtReturn GaPlayerEntity::onMouseUp( EvtID ID, const OsEventInputMouse& Event )
{
	return evtRET_PASS;
}
