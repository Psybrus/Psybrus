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

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaElementComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaElementComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	Colour_ = RsColour( Object["colour"].asCString() );
	MaxSpeed_ = (BcReal)Object["maxspeed"].asDouble();
	Direction_ = (BcReal)Object["direction"].asDouble();
	Radius_ = (BcReal)Object["radius"].asDouble();
	Rotation_ = BcVec3d( 0.0f, 0.0f, 0.0f );

	AngularVelocity_ = BcRandom::Global.randVec3Normal();

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

	// Update physics.
	BcMat4d Matrix( getParentEntity()->getMatrix() );
	BcMat4d ScaleMatrix;
	ScaleMatrix.scale( BcVec3d( Radius_, Radius_, Radius_ ) );
	BcVec3d Position = Matrix.translation();
	Rotation_ += AngularVelocity_ * Tick;
	Matrix.rotation( Rotation_ );

	getParentEntity()->setMatrix( Matrix * ScaleMatrix );
}

//////////////////////////////////////////////////////////////////////////
// GaElementComponent
//virtual
void GaElementComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Get first model component.
	Model_ = Parent->getComponentByType< ScnModelComponent >( 0 );

	// Get material from model.
	Material_ = Model_->getMaterialComponent( "element" );
	MaterialColourParam_ = Material_->findParameter( "uColour" );

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

