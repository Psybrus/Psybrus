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
	GaFoodEntity( const BcVec2d& Position );
	virtual ~GaFoodEntity();

	void eat( BcReal Tick );

	virtual BcBool isAlive();
	BcReal getHealthFraction();

	virtual void update( BcReal Tick );
	virtual void render( ScnCanvasComponentRef Canvas );
	
private:
	BcReal StartingHealth_;
	BcReal Health_;

	ScnMaterialComponentRef MaterialComponent_;
	ScnMaterialComponentRef ShadowMaterialComponent_;
};

#endif
