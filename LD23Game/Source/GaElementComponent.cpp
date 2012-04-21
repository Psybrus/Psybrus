/**************************************************************************
*
* File:		GaElementComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	29/12/11	
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
	Rotation_ = 0.0f;
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
	BcVec3d Position = Matrix.translation();
	Rotation_ += Tick;
	Matrix.rotation( BcVec3d( 0.0f, Rotation_, 0.0f ) );

	getParentEntity()->setMatrix( Matrix );
}

//////////////////////////////////////////////////////////////////////////
// GaElementComponent
//virtual
void GaElementComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Find model component.
	for( BcU32 Idx = 0; Idx < Parent->getNoofComponents(); ++Idx )
	{
		ScnComponentRef Component( Parent->getComponent( Idx ) );

		if( Component->isTypeOf< ScnModelComponent >() )
		{
			Model_ = Component;
		}
	}

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

