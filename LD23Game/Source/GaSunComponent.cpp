/**************************************************************************
*
* File:		GaSunComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	22/04/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "GaSunComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaSunComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaSunComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );
	
	Colour_ = RsColour( Object["colour"].asCString() );
	Radius_ = BcReal( Object["radius"].asDouble() );
	RadiusMult_ = BcReal( Object["radiusmult"].asDouble() );
	RotationMult_ = BcReal( Object["rotationmult"].asDouble() );
	RotationSpeed_ = BcReal( Object["rotationspeed"].asDouble() );
	Rotation_ = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// GaSunComponent
//virtual
void GaSunComponent::update( BcReal Tick )
{
	Super::update( Tick );

	Rotation_ += Tick * RotationSpeed_;

	// Setup scale and stuff.
	BcReal ScaledRadius = Radius_;
	BcReal ScaledRotation = Rotation_;
	for( BcU32 Idx = 0; Idx < Models_.size(); ++Idx )
	{
		Materials_[ Idx ]->setParameter( MaterialColourParams_[ Idx ], Colour_ );

		BcMat4d Rotation;
		BcMat4d Scale;

		if( Idx != 0 )
		{
			Scale.scale( BcVec3d( ScaledRadius, ScaledRadius, ScaledRadius ) );
			Rotation.rotation( BcVec3d( BcPIDIV2 * 0.75f, ScaledRotation, 0.0f ) );
			Models_[ Idx ]->setTransform( 0, Scale * Rotation );
		}
		else
		{
			BcReal SkyRadius = 512.0f;
			Scale.scale( BcVec3d( SkyRadius, SkyRadius, SkyRadius ) );
			Rotation.rotation( BcVec3d( BcPIDIV2 * 0.75f, ScaledRotation, 0.0f ) );
			Models_[ Idx ]->setTransform( 0, Scale * Rotation );
		}

		ScaledRotation *= RotationMult_;
		ScaledRadius *= RadiusMult_;
	}
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaSunComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Grab all attached models.
	for( BcU32 Idx = 0; Parent->getComponentByType< ScnModelComponent >( Idx ).isValid(); ++Idx )
	{
		ScnModelComponentRef ModelRef = Parent->getComponentByType< ScnModelComponent >( Idx );
		Models_.push_back( ModelRef );
		Materials_.push_back( Models_[ Idx ]->getMaterialComponent( 0 ) );
		MaterialColourParams_.push_back( Materials_[ Idx ]->findParameter( "uColour" ) );
	}

	ScnSoundEmitterComponentRef SoundEmitter = Parent->getComponentByType< ScnSoundEmitterComponent >( 0 );
	if( SoundEmitter.isValid() )
	{
		SoundEmitter->play( "game", "music" );
	}
	
	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaSunComponent::onDetach( ScnEntityWeakRef Parent )
{
	Models_.clear();
	Materials_.clear();
	MaterialColourParams_.clear();

	// Don't forget to detach!
	Super::onDetach( Parent );
}
