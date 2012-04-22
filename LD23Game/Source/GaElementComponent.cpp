/**************************************************************************
*
* File:		GaElementComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	21/04/12
* Description:
*		Ball component.
*		
*
*
* 
**************************************************************************/

#include "GaElementComponent.h"

#include "GaGameComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaElementComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaElementComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	Colour_ = RsColour( Object["colour"].asCString() );
	ShadowColour_ = RsColour( Object["shadowcolour"].asCString() );
	MaxSpeed_ = (BcReal)Object["maxspeed"].asDouble();
	Direction_ = (BcReal)Object["direction"].asDouble();
	Radius_ = (BcReal)Object["radius"].asDouble();
	Rotation_ = BcVec3d( 0.0f, 0.0f, 0.0f );

	AngularVelocity_ = BcRandom::Global.randVec3Normal() * 3.0f;

	Json::Value FuseTypeValue = Object.get( "fusetype", Json::nullValue );
	Json::Value ReplaceTypeValue = Object.get( "replacetype", Json::nullValue );
	Json::Value RespawnTypeValue = Object.get( "respawntype", Json::nullValue );
	FuseType_ = FuseTypeValue.type() != Json::nullValue ? FuseTypeValue.asCString() : BcName::INVALID;
	ReplaceType_ = ReplaceTypeValue.type() != Json::nullValue ? ReplaceTypeValue.asCString() : BcName::INVALID;
	RespawnType_ = RespawnTypeValue.type() != Json::nullValue ? RespawnTypeValue.asCString() : BcName::INVALID;
}

//////////////////////////////////////////////////////////////////////////
// GaElementComponent
//virtual
void GaElementComponent::update( BcReal Tick )
{
	Super::update( Tick );

	// Set material colour up.
	Material_->setParameter( MaterialColourParam_, Colour_ );
	ShadowMaterial_->setParameter( ShadowMaterialColourParam_, Colour_ );

	// Update physics.
	BcMat4d RotationMatrix;
	Rotation_ += AngularVelocity_ * Tick;
	RotationMatrix.rotation( Rotation_ );
	Model_->setTransform( 0, RotationMatrix );

	BcMat4d ShadowMatrix;
	ShadowMatrix.scale( BcVec3d( 4.0f, 1.0f, 4.0f ) );
	ShadowMatrix.translation( BcVec3d( 0.0f, -1.0f, 0.0f ) );
	ShadowModel_->setTransform( 0, ShadowMatrix );
}

//////////////////////////////////////////////////////////////////////////
// GaElementComponent
//virtual
void GaElementComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Model stuff.
	Model_ = Parent->getComponentByType< ScnModelComponent >( 0 );
	Material_ = Model_->getMaterialComponent( 0 );
	MaterialColourParam_ = Material_->findParameter( "uColour" );

	// Model stuff.
	ShadowModel_ = Parent->getComponentByType< ScnModelComponent >( 1 );
	ShadowMaterial_ = ShadowModel_->getMaterialComponent( 0 );
	ShadowMaterialColourParam_ = ShadowMaterial_->findParameter( "uColour" );

	// Set visible mask stuff.
	BcReal ScaleFactor = 0.5f / WORLD_SIZE;
	BcMat4d VisibleMapMatrix;
	VisibleMapMatrix.row0( BcVec4d( ScaleFactor,	0.0f,			0.0f,			0.0f ) );
	VisibleMapMatrix.row1( BcVec4d( 0.0f,			0.0f,			-ScaleFactor,	0.0f ) );
	VisibleMapMatrix.row2( BcVec4d( 0.0f,			ScaleFactor,	0.0f,			0.0f ) );
	VisibleMapMatrix.row3( BcVec4d( 0.5f,			0.5f,			0.5f,			1.0f ) );
	Material_->setParameter( Material_->findParameter( "uVisibleMapMatrix" ), VisibleMapMatrix );
	ShadowMaterial_->setParameter( ShadowMaterial_->findParameter( "uVisibleMapMatrix" ), VisibleMapMatrix );

	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// GaElementComponent
//virtual
void GaElementComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Don't forget to detach!
	Super::onDetach( Parent );
}

