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

#ifndef __GAFOODENTITY_H__
#define __GAFOODENTITY_H__

#include "Psybrus.h"

#include "GaEntity.h"

////////////////////////////////////////////////////////////////////////////////
// GaFoodEntity
class GaFoodEntity:
	public GaEntity
{
public:
	GaFoodEntity( const BcMat4d& Projection );
	virtual ~GaFoodEntity();

	virtual void update( BcReal Tick );
	virtual void render( ScnCanvasRef Canvas );
	
private:
	BcVec2d Position_;
};

#endif
