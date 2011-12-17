/**************************************************************************
*
* File:		GaFoodEntity.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Food.
*		
*
*
* 
**************************************************************************/

#include "GaFoodEntity.h"

////////////////////////////////////////////////////////////////////////////////
// GaFoodEntity
GaFoodEntity::GaFoodEntity( const BcMat4d& Projection )
{
	Projection_ = Projection;

}

////////////////////////////////////////////////////////////////////////////////
// GaFoodEntity
//virtual
GaFoodEntity::~GaFoodEntity()
{

}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaFoodEntity::update( BcReal Tick )
{

}

////////////////////////////////////////////////////////////////////////////////
// render
//virtual
void GaFoodEntity::render( ScnCanvasRef Canvas )
{

}
