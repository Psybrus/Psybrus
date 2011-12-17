/**************************************************************************
*
* File:		GaSwarmEntity.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Swarm.
*		
*
*
* 
**************************************************************************/

#ifndef __GAENTITY_H__
#define __GAENTITY_H__

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// GaEntity
class GaEntity
{
public:
	GaEntity();
	virtual ~GaEntity();

	virtual void update( BcReal Tick );
	virtual void render( ScnCanvasRef Canvas );

protected:
	BcMat4d Projection_;
	BcVec2d Position_;

};

#endif
