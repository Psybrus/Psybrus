/**************************************************************************
*
* File:		GaPlayerComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	29/12/11	
* Description:
*		Example user component.
*		
*
*
* 
**************************************************************************/

#include "GaPlayerComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaPlayerComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaPlayerComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );
	
	Position_ = BcVec3d( 0.0f, 0.0f, 0.0f );
	Rotation_ = 0.0f;
	MoveForward_ = BcFalse;
	MoveBackward_ = BcFalse;
	MoveLeft_ = BcFalse;
	MoveRight_ = BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// GaPlayerComponent
//virtual
void GaPlayerComponent::update( BcReal Tick )
{
	BcReal RotationSpeed = 1.0f * Tick;
	BcReal MoveSpeed = 8.0f;
	BcReal MoveDirection = 0.0f;
	BcVec3d MoveVector = BcVec3d( BcCos( Rotation_ ), -BcSin( Rotation_ ), 0.0f );
	BcVec3d SideVector = BcVec3d( BcCos( Rotation_ + BcPIDIV2 ), -BcSin( Rotation_ + BcPIDIV2 ), 0.0f );
	BcVec3d TargetPosition = Position_;
	BcVec3d TargetCastPosition = Position_;
	if( MoveForward_ )
	{
		MoveDirection = 1.0f;
	}
	if( MoveBackward_ )
	{
		MoveDirection = -1.0f;
	}
	if( MoveLeft_ )
	{
		Rotation_ += RotationSpeed;
	}
	if( MoveRight_ )
	{
		Rotation_ -= RotationSpeed;
	}

	BcReal PlayerRadius = 0.25f;
	BcReal LineCheckRadius = 1.0f;

	// Collide against walls.
	BcVec3d AppliedMoveVector = MoveVector * MoveSpeed * MoveDirection * Tick;
	BcVec3d RadiusMoveVector = MoveVector * LineCheckRadius * MoveDirection;
	
	BcU32 Tries = 3;
	BcBSPInfo BSPInfo;
	do
	{
		BcVec3d SidePositionA = ( Position_ - SideVector * PlayerRadius );
		BcVec3d SidePositionB = ( Position_ + SideVector * PlayerRadius );
		BcVec3d SidePositionC = ( Position_ + MoveVector * PlayerRadius * 2.0f );
		BcBSPPointInfo BSPPointInfoA;
		BcBSPPointInfo BSPPointInfoB;
		BcBSPPointInfo BSPPointInfoC;
		BcBool PointAIntersection = BSP_->lineIntersection( SidePositionA, SidePositionA + RadiusMoveVector, &BSPPointInfoA );
		BcBool PointBIntersection = BSP_->lineIntersection( SidePositionB, SidePositionB + RadiusMoveVector, &BSPPointInfoB );
		BcBool PointCIntersection = BSP_->lineIntersection( SidePositionC, SidePositionC + RadiusMoveVector, &BSPPointInfoC );

		Canvas_->drawLine( BcVec2d( SidePositionA.x(), SidePositionA.y() ), BcVec2d( BSPPointInfoA.Point_.x(), BSPPointInfoA.Point_.y() ), RsColour::GREEN, 3 );
		Canvas_->drawLine( BcVec2d( SidePositionB.x(), SidePositionB.y() ), BcVec2d( BSPPointInfoB.Point_.x(), BSPPointInfoB.Point_.y() ), RsColour::GREEN, 3 );
		Canvas_->drawLine( BcVec2d( SidePositionC.x(), SidePositionC.y() ), BcVec2d( BSPPointInfoC.Point_.x(), BSPPointInfoC.Point_.y() ), RsColour::GREEN, 3 );
		
		if( PointAIntersection )
		{
			BcVec3d Normal = BSPPointInfoA.Plane_.normal();
			BcReal MoveAwaySpeed = AppliedMoveVector.dot( Normal );
			AppliedMoveVector -= Normal * MoveAwaySpeed * 1.001f;
		}

		if( PointBIntersection )
		{
			BcVec3d Normal = BSPPointInfoB.Plane_.normal();
			BcReal MoveAwaySpeed = AppliedMoveVector.dot( Normal );
			AppliedMoveVector -= Normal * MoveAwaySpeed * 1.001f;
		}

		if( PointCIntersection )
		{
			BcVec3d Normal = BSPPointInfoC.Plane_.normal();
			BcReal MoveAwaySpeed = AppliedMoveVector.dot( Normal );
			AppliedMoveVector -= Normal * MoveAwaySpeed * 1.001f;
		}

		// Move to position.
		TargetPosition = Position_ + AppliedMoveVector;

	}	
	while( !BSP_->checkPointFront( TargetPosition, PlayerRadius, &BSPInfo ) && --Tries != 0 );

	BcVec2d Position2D = BcVec2d( Position_.x(), Position_.y() );
	BcVec2d BoxSize2D = BcVec2d( PlayerRadius, PlayerRadius );

	Canvas_->drawLineBox( Position2D - BoxSize2D, Position2D + BoxSize2D, RsColour::GREEN, 3 );

	if( BSP_->checkPointFront( TargetPosition, PlayerRadius, &BSPInfo ) )
	{
		Position_ = TargetPosition;
	}

	// Setup camera.
	BcMat4d Transform;
	Transform.lookAt( Position_, Position_ + MoveVector, BcVec3d( 0.0f, 0.0f, 1.0f ) );
	Transform.inverse();
	getParentEntity()->setMatrix( Transform );
}

//////////////////////////////////////////////////////////////////////////
// GaPlayerComponent
//virtual
void GaPlayerComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Don't forget to attach!
	Super::onAttach( Parent );
	
	OsEventInputKeyboard::Delegate OnKeyboardEvent = OsEventInputKeyboard::Delegate::bind< GaPlayerComponent, &GaPlayerComponent::onKeyboardEvent >( this );
	
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, OnKeyboardEvent );
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYUP, OnKeyboardEvent );

	// Find world entity and cache the BSP.
	WorldEntity_ = ScnCore::pImpl()->findEntity( "WorldEntity_0" );
	Canvas_ = WorldEntity_->getComponentByType< ScnCanvasComponent >( 0 );
	BSP_ = WorldEntity_->getComponentByType< GaWorldBSPComponent >( 0 );
	Pressure_ = WorldEntity_->getComponentByType< GaWorldPressureComponent >( 0 );
}

//////////////////////////////////////////////////////////////////////////
// GaPlayerComponent
//virtual
void GaPlayerComponent::onDetach( ScnEntityWeakRef Parent )
{
	OsCore::pImpl()->unsubscribeAll( this );

	WorldEntity_ = NULL;
	Canvas_ = NULL;
	BSP_ = NULL;
	Pressure_ = NULL;

	// Don't forget to detach!
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onKeyboardEvent
eEvtReturn GaPlayerComponent::onKeyboardEvent( EvtID ID, const OsEventInputKeyboard& Event )
{
	if( ID == osEVT_INPUT_KEYDOWN || ID == osEVT_INPUT_KEYUP )
	{
		BcBool State = ID == osEVT_INPUT_KEYDOWN;
		switch( Event.KeyCode_ )
		{
		case OsEventInputKeyboard::KEYCODE_UP:
			MoveForward_ = State;
			break;

		case OsEventInputKeyboard::KEYCODE_DOWN:
			MoveBackward_ = State;
			break;

		case OsEventInputKeyboard::KEYCODE_LEFT:
			MoveLeft_ = State;
			break;

		case OsEventInputKeyboard::KEYCODE_RIGHT:
			MoveRight_ = State;
			break;

		case OsEventInputKeyboard::KEYCODE_SPACE:
			Position_ = BcVec3d( 0.0f, 0.0f, 0.0f );
			break;
		}
	}

	return evtRET_PASS;
}
