/**************************************************************************
*
* File:		GaLevelComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	15/12/12		
* Description:
*		Level component.
*		
*
*
* 
**************************************************************************/

#include "GaLevelComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaLevelComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaLevelComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );
}

//////////////////////////////////////////////////////////////////////////
// GaLevelComponent
//virtual
void GaLevelComponent::update( BcReal Tick )
{
	Super::update( Tick );

	static BcReal Ticker = 0.0f;
	Ticker += Tick * 2.0f;
	BcMat4d Matrix;
	//Matrix.rotation( BcVec3d( Ticker * 0.3, Ticker * 0.1, Ticker * 0.2 ) );
	getParentEntity()->setMatrix( Matrix );

	LevelMaterial_->setParameter( UVScrollingParam_, BcVec4d( 0.0f, -Ticker, 0.0f, 0.0f ) );
}

//////////////////////////////////////////////////////////////////////////
// GaLevelComponent
//virtual
void GaLevelComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Grab modwel and it's material, and it's UV scrolling parameter.
	ScnModelComponentRef Model = Parent->getComponentByType< ScnModelComponent >( 0 );
	LevelMaterial_ = Model->getMaterialComponent( 0 );
	UVScrollingParam_ = LevelMaterial_->findParameter( "uUVScrolling" );
	
	// Grab the canvas.
	Canvas_ = Parent->getComponentAnyParentByType< ScnCanvasComponent >( 0 );
	
	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// GaLevelComponent
//virtual
void GaLevelComponent::onDetach( ScnEntityWeakRef Parent )
{
	LevelMaterial_ = NULL;

	// Don't forget to detach!
	Super::onDetach( Parent );
}

