/**************************************************************************
*
* File:		GaPlayerComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		Player component.
*		
*
*
* 
**************************************************************************/

#include "GaPlayerComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaPlayerComponent );

BCREFLECTION_EMPTY_REGISTER( GaPlayerComponent );
/*
BCREFLECTION_DERIVED_BEGIN( ScnComponent, GaPlayerComponent )
	BCREFLECTION_MEMBER( BcName,							Name_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								Index_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( CsPackage,							pPackage_,						bcRFF_POINTER | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								RefCount_,						bcRFF_DEFAULT | bcRFF_TRANSIENT ),
BCREFLECTION_DERIVED_END();
*/

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

	DoShot_ = BcFalse;
	RateOfShot_ = 0.4f;
	ShotTick_ = 0.0f;
	HasWeapon_ = BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// GaPlayerComponent
//virtual
void GaPlayerComponent::update( BcF32 Tick )
{
	// Mouse update.
	Yaw_ += -MouseDelta_.x() * Tick * 0.1f;
	Pitch_ += -MouseDelta_.y() * Tick * 0.1f;
	Pitch_ = BcClamp( Pitch_, -BcPIDIV2 + ( BcPIDIV2 * 0.125f ), BcPIDIV2 - ( BcPIDIV2 * 0.125f ) );
	MouseDelta_ = BcVec2d( 0.0f, 0.0f );

	BcF32 RotationSpeed = 1.0f * Tick;
	BcF32 MoveSpeed = DoRun_ ? 4.0f : 2.0f;
	BcF32 MoveDirection = 0.0f;
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

	if( DoPulse_ == BcTrue )
	{
		doShot( ViewVector, 0.2f, 0.1f, 1.0f );
		DoPulse_ = BcFalse;
	}

	if( DoShot_ == BcTrue && HasWeapon_ )
	{
		if( ShotTick_ >= RateOfShot_ )
		{
			BcVec3d ImpactPosition = doShot( ViewVector, 0.1f, 4.0f, 32.0f );
			if( BSP_->killEnemy( ImpactPosition, 4.0f ) )
			{
				Pressure_->setSample( ImpactPosition, -512.0f );
			}
			
			ShotTick_ -= 1.0f;
		}

		ShotTick_ += Tick;
	}
	else
	{
		if( ShotTick_ > RateOfShot_ )
		{
			ShotTick_ = -1.0f;
		}
		else
		{
			ShotTick_ += Tick;
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

	// Setup ears.
	BcVec3d EarLVector = BcVec3d( 1.0f, 0.0f, 0.0f );
	BcVec3d EarRVector = BcVec3d( 1.0f, 0.0f, 0.0f );
	BcF32 Offset = 0.0f;
	for( BcU32 Idx = 0; Idx < 4; ++Idx )
	{
		EarLVectors_[ Idx ] = EarLVector * RotationMatrix;
		EarRVectors_[ Idx ] = EarRVector * RotationMatrix;

		EarLVectors_[ Idx ] += SideVector * Offset;
		EarLVectors_[ Idx ].normalise();
	
		EarRVectors_[ Idx ] -= SideVector * Offset;
		EarRVectors_[ Idx ].normalise();

		Offset += 0.05f;
	}
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
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseEvent );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEUP, OnMouseEvent );
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, OnKeyboardEvent );
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYUP, OnKeyboardEvent );

	// Cache world stuff we need.
	ScnEntityRef WorldEntity = ScnCore::pImpl()->findEntity( "WorldEntity_0" );
	Canvas_ = WorldEntity->getComponentAnyParentByType< ScnCanvasComponent >( 0 );
	BSP_ = WorldEntity->getComponentByType< GaWorldBSPComponent >( 0 );
	Pressure_ = WorldEntity->getComponentByType< GaWorldPressureComponent >( 0 );

	// Subscribe to reset event.
	GaWorldResetEvent::Delegate OnResetEvent = GaWorldResetEvent::Delegate::bind< GaPlayerComponent, &GaPlayerComponent::onReset >( this );
	WorldEntity->subscribe( gaEVT_CORE_RESET, OnResetEvent );

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
	
	ScnEntityRef WorldEntity = ScnCore::pImpl()->findEntity( "WorldEntity_0" );
	WorldEntity->unsubscribeAll( this );
	
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
		case 'W':
			MoveForward_ = State;
			break;

		case OsEventInputKeyboard::KEYCODE_DOWN:
		case 'S':
			MoveBackward_ = State;
			break;

		case OsEventInputKeyboard::KEYCODE_LEFT:
		case 'A':
			MoveLeft_ = State;
			break;

		case OsEventInputKeyboard::KEYCODE_RIGHT:
		case 'D':
			MoveRight_ = State;
			break;

		case OsEventInputKeyboard::KEYCODE_SHIFT:
		case OsEventInputKeyboard::KEYCODE_LSHIFT:
		case OsEventInputKeyboard::KEYCODE_RSHIFT:
			DoRun_ = State;
			break;

		case OsEventInputKeyboard::KEYCODE_SPACE:
			//Pawn_->setPosition( BcVec3d( 0.0f, 0.0f, 0.0f ) );
			break;

		case OsEventInputKeyboard::KEYCODE_ESCAPE:
			exit(0);
			break;

		case OsEventInputKeyboard::KEYCODE_CONTROL:
			if( ID == osEVT_INPUT_KEYDOWN )
			{
				DoPulse_ = BcTrue;
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
	switch( ID )
	{
	case osEVT_INPUT_MOUSEMOVE:
		MouseDelta_.set( Event.MouseDX_, Event.MouseDY_ );
		break;

	case osEVT_INPUT_MOUSEDOWN:
		DoShot_ = BcTrue;
		if( ShotTick_ < 0.0f )
		{
			ShotTick_ = RateOfShot_;
		}
		break;

	case osEVT_INPUT_MOUSEUP:
		DoShot_ = BcFalse;
		break;
	}
	
	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// doShot
BcVec3d GaPlayerComponent::doShot( const BcVec3d& Direction, BcF32 TrailPower, BcF32 MuzzlePower, BcF32 ImpactPower )
{
	BcVec3d Position = getParentEntity()->getPosition();
	BcVec3d Target = Position + Direction * 256.0f;

	BcBSPPointInfo BSPPointInfo;
	if( BSP_->lineIntersection( Position, Target, &BSPPointInfo ) )
	{
		
	}
	else
	{
		BcPlane Floor( BcVec3d( 0.0f, 0.0f,  1.0f ), 4.0f );
		BcPlane Ceil( BcVec3d( 0.0f, 0.0f, -1.0f ), 4.0f );
		BcF32 Dist;
		Floor.lineIntersection( Position, Target, Dist, BSPPointInfo.Point_ );
		Ceil.lineIntersection( Position, Target, Dist, BSPPointInfo.Point_ );
	}

	// Draw pressure into the scene at the start and end points.
	if( MuzzlePower > 0.0f )
	{
		Pressure_->setSample( BSPPointInfo.Point_, MuzzlePower );
	}

	if( ImpactPower > 0.0f )
	{
		Pressure_->setSample( BSPPointInfo.Point_, ImpactPower );
	}

	// Trace a line through the scene.
	BcF32 Distance = ( BSPPointInfo.Point_ - Position ).magnitude() * 2.0f;
	BcVec3d Point = Position;
	BcVec3d StepVec = ( BSPPointInfo.Point_ - Position ) / Distance;
	for( BcU32 Idx = 0; Idx < BcU32( Distance ); ++Idx )
	{
		Pressure_->setSample( Point, TrailPower );
		Point += StepVec;
	}

	return BSPPointInfo.Point_;
}

//////////////////////////////////////////////////////////////////////////
// onReset
eEvtReturn GaPlayerComponent::onReset( EvtID ID, const GaWorldResetEvent& Event )
{
	HasWeapon_ = Event.HasWeapon_;

	return evtRET_PASS;
}

