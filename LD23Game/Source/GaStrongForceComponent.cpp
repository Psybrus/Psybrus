/**************************************************************************
*
* File:		GaStrongForceComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	21/04/12
* Description:
*		Ball component.
*		
*
*
* 
**************************************************************************/

#include "GaStrongForceComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaStrongForceComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaStrongForceComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );
	
	Colour_ = RsColour( Object["colour"].asCString() );
	Position_ = BcVec3d( 0.0f, 0.0f, 0.0f );

	IsCharging_ = BcFalse;

	Radius_ = 2.0f;
	TargetRadius_ = 2.0f;
}

//////////////////////////////////////////////////////////////////////////
// GaStrongForceComponent
//virtual
void GaStrongForceComponent::update( BcReal Tick )
{
	Super::update( Tick );

	RsColour InactiveColour( 0.0f, 0.0f, 0.0f, 1.0f );
	RsColour ActiveColour( 1.0f, 1.0f, 1.0f, 1.0f );

	// Set material colour up.
	if( IsCharging_ )
	{
		Material_->setParameter( MaterialColourParam_, Colour_ * ActiveColour );
	}
	else
	{
		Material_->setParameter( MaterialColourParam_, Colour_ * InactiveColour );
	}

	if( IsCharging_ )
	{
		Radius_ = BcLerp( Radius_, TargetRadius_, Tick );
	}
	else
	{
		Radius_ = BcLerp( Radius_, TargetRadius_, Tick * 10.0f );
	}

	BcMat4d Matrix;
	Matrix.scale( BcVec3d( Radius_, Radius_, Radius_ ) );
	Matrix.translation( Position_ );
	getParentEntity()->setMatrix( Matrix );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaStrongForceComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Find the view (used for mouse unprojection).
	ScnEntityRef ScreenEntity;
	if( CsCore::pImpl()->requestResource( BcName::NONE, "ScreenEntity_0", ScreenEntity ) )
	{
		View_ = ScreenEntity->getComponentByType< ScnViewComponent >( 0 );
	}
	
	// Get first model component.
	Model_ = Parent->getComponentByType< ScnModelComponent >( 0 );

	// Get material from model.
	Material_ = Model_->getMaterialComponent( "strongforce" );
	MaterialColourParam_ = Material_->findParameter( "uColour" );

	OsEventInputMouse::Delegate OnMouseEvent = OsEventInputMouse::Delegate::bind< GaStrongForceComponent, &GaStrongForceComponent::onMouseEvent >( this );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, OnMouseEvent );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseEvent );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEUP, OnMouseEvent );

	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaStrongForceComponent::onDetach( ScnEntityWeakRef Parent )
{
	OsCore::pImpl()->unsubscribeAll( this );

	View_ = NULL;
	Model_ = NULL;
	Material_ = NULL;

	// Don't forget to detach!
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onMouseEvent
eEvtReturn GaStrongForceComponent::onMouseEvent( EvtID ID, const OsEventInputMouse& Event )
{
	BcVec2d ScreenPos( Event.MouseX_, Event.MouseY_ );
	BcVec3d Near, Far;
	View_->getWorldPosition( ScreenPos, Near, Far );

	BcPlane GroundPlane( BcVec3d( 0.0f, 1.0f, 0.0f ), 0.0f );
	BcVec3d Intersection; 
	BcBool HaveIntersection = GroundPlane.lineIntersection( Near, Far, Intersection );

	switch( ID )
	{
	case osEVT_INPUT_MOUSEMOVE:
		{
			if( HaveIntersection && IsCharging_ == BcFalse )
			{
				Position_ = Intersection;
			}
		}
		break;

	case osEVT_INPUT_MOUSEDOWN:
		{
			if( Event.ButtonCode_ == 0 )
			{
				IsActive_ = BcTrue;
				IsCharging_ = BcFalse;
			}
			else if( Event.ButtonCode_ == 1 )
			{
				TargetRadius_ = 8.0f;
				IsCharging_ = BcTrue;
			}
		}
		break;

	case osEVT_INPUT_MOUSEUP:
		{
			if( Event.ButtonCode_ == 0 )
			{
				IsActive_ = BcFalse;
			}
			else if( Event.ButtonCode_ == 1 )
			{
				TargetRadius_ = 2.0f;
				IsCharging_ = BcFalse;
			}
		}
		break;
	}

	return evtRET_PASS;
}