/**************************************************************************
*
* File:		GaSwarmEntity.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Swarm.
*		
*
*
* 
**************************************************************************/

#include "GaSwarmEntity.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
GaSwarmEntity::GaSwarmEntity( const BcMat4d& Projection )
{
	Projection_ = Projection;
	Position_ = BcVec2d( 0.0f, 0.0f );

}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaSwarmEntity::~GaSwarmEntity()
{

}

////////////////////////////////////////////////////////////////////////////////
// update
void GaSwarmEntity::update( BcReal Tick )
{
	/*
	if( LerpDelta_ < 1.0f )
	{
		BcReal LerpAmount = ( MovementSpeed_ / ( EndPosition_ - StartPosition_ ).magnitude() ) * Tick;
		LerpDelta_ = BcMin( LerpDelta_ + LerpAmount, 1.0f );

		Position_.lerp( StartPosition_, EndPosition_, LerpDelta_ );
	}
	*/
}

////////////////////////////////////////////////////////////////////////////////
// render
void GaSwarmEntity::render( ScnCanvasRef Canvas )
{
	//BcVec2d HalfSize( 16.0f, 16.0f );
	//Canvas->drawBox( Position_ - HalfSize, Position_ + HalfSize, RsColour::GREEN, 0 );
}

