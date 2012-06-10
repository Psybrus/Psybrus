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
	
	// Set the move.
	BcVec3d AppliedMoveVector = MoveVector * MoveSpeed * MoveDirection;
	Pawn_->setMove( AppliedMoveVector );

	// Set look at.
	BcMat4d Transform;
	Transform.lookAt( BcVec3d( 0.0f, 0.0f, 0.0f ), MoveVector, BcVec3d( 0.0f, 0.0f, 1.0f ) );
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

	// Cache world stuff we need.
	ScnEntityRef WorldEntity = ScnCore::pImpl()->findEntity( "WorldEntity_0" );
	Canvas_ = WorldEntity->getComponentByType< ScnCanvasComponent >( 0 );
	BSP_ = WorldEntity->getComponentByType< GaWorldBSPComponent >( 0 );
	Pressure_ = WorldEntity->getComponentByType< GaWorldPressureComponent >( 0 );

	// Grab pawn.
	Pawn_ = Parent->getComponentByType< GaPawnComponent >( 0 );
}

//////////////////////////////////////////////////////////////////////////
// GaPlayerComponent
//virtual
void GaPlayerComponent::onDetach( ScnEntityWeakRef Parent )
{
	OsCore::pImpl()->unsubscribeAll( this );

	Canvas_ = NULL;
	BSP_ = NULL;
	Pressure_ = NULL;
	Pawn_ = NULL;

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
			Pawn_->setPosition( BcVec3d( 0.0f, 0.0f, 0.0f ) );
			break;
		}
	}

	return evtRET_PASS;
}
