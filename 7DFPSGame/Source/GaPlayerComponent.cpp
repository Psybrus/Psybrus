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
	
	Yaw_ = 0.0f;
	Pitch_ = 0.0f;
	MoveForward_ = BcFalse;
	MoveBackward_ = BcFalse;
	MoveLeft_ = BcFalse;
	MoveRight_ = BcFalse;

	DoRun_ = BcFalse;
	DoPulse_ = BcFalse;
	PulseTick_ = -1.0f;
}

//////////////////////////////////////////////////////////////////////////
// GaPlayerComponent
//virtual
void GaPlayerComponent::update( BcReal Tick )
{
	// Mouse update.
	Yaw_ += -MouseDelta_.x() * Tick * 0.1f;
	Pitch_ += -MouseDelta_.y() * Tick * 0.1f;
	Pitch_ = BcClamp( Pitch_, -BcPIDIV2 + ( BcPIDIV2 * 0.125f ), BcPIDIV2 - ( BcPIDIV2 * 0.125f ) );
	MouseDelta_ = BcVec2d( 0.0f, 0.0f );

	BcReal RotationSpeed = 1.0f * Tick;
	BcReal MoveSpeed = DoRun_ ? 4.0f : 2.0f;
	BcReal MoveDirection = 0.0f;
	BcVec3d ViewVector = BcVec3d( 1.0f, 0.0f, 0.0f );
	BcMat4d RotationMatrix;
	RotationMatrix.rotation( BcVec3d( 0.0f, Pitch_, Yaw_ ) );
	ViewVector = ViewVector * RotationMatrix;
	BcVec3d SideVector = ViewVector.cross( BcVec3d( 0.0f, 0.0f, 1.0f ) );
	BcVec3d MoveVector;

	if( MoveForward_ )
	{
		MoveVector = MoveVector + ViewVector;
	}
	if( MoveBackward_ )
	{
		MoveVector = MoveVector - ViewVector;
	}
	if( MoveLeft_ )
	{
		MoveVector = MoveVector + SideVector;
	}
	if( MoveRight_ )
	{
		MoveVector = MoveVector - SideVector;
	}

	if( DoPulse_ == BcTrue || PulseTick_ > 0.0f )
	{
		BcVec3d Position( getParentEntity()->getPosition() );
		Position.z( -1.0f );
		Pressure_->setSample( Position, BcSin( PulseTick_ ) * 16.0f );
		PulseTick_ += Tick * 8.0f;

		if( PulseTick_ > BcPIMUL2 )
		{
			DoPulse_ = BcFalse;
			PulseTick_ = -1.0f;
		}
	}
	
	// Set the move.
	BcVec3d AppliedMoveVector = MoveVector;
	AppliedMoveVector.z( 0.0f );
	AppliedMoveVector = AppliedMoveVector.normal() * MoveSpeed;
	Pawn_->setMove( AppliedMoveVector );

	// Set look at.
	BcMat4d Transform;
	Transform.lookAt( BcVec3d( 0.0f, 0.0f, 0.0f ), ViewVector, BcVec3d( 0.0f, 0.0f, 1.0f ) );
	Transform.inverse();
	getParentEntity()->setMatrix( Transform );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaPlayerComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Don't forget to attach!
	Super::onAttach( Parent );
	
	OsEventInputKeyboard::Delegate OnKeyboardEvent = OsEventInputKeyboard::Delegate::bind< GaPlayerComponent, &GaPlayerComponent::onKeyboardEvent >( this );
	OsEventInputMouse::Delegate OnMouseEvent = OsEventInputMouse::Delegate::bind< GaPlayerComponent, &GaPlayerComponent::onMouseEvent >( this );
	
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, OnMouseEvent );
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
// onDetach
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
		case 'W':
			MoveForward_ = State;
			break;

		case 'S':
			MoveBackward_ = State;
			break;

		case 'A':
			MoveLeft_ = State;
			break;

		case 'D':
			MoveRight_ = State;
			break;

		case OsEventInputKeyboard::KEYCODE_SHIFT:
			DoRun_ = State;
			break;

		case OsEventInputKeyboard::KEYCODE_SPACE:
			Pawn_->setPosition( BcVec3d( 0.0f, 0.0f, 0.0f ) );
			break;

		case OsEventInputKeyboard::KEYCODE_CONTROL:
			if( !DoPulse_ )
			{
				DoPulse_ = State;
				PulseTick_ = State ? 0.0f : PulseTick_;
			}
			break;
		}
	}

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onMouseEvent
eEvtReturn GaPlayerComponent::onMouseEvent( EvtID ID, const OsEventInputMouse& Event )
{
	MouseDelta_.set( Event.MouseDX_, Event.MouseDY_ );
	return evtRET_PASS;
}
