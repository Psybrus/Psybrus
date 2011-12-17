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

#ifndef __GASWAWM_H__
#define __GASWAWM_H__

#include "Psybrus.h"

#include "GaEntity.h"

////////////////////////////////////////////////////////////////////////////////
// GaSwarmEntity
class GaSwarmEntity:
	public GaEntity
{
public:
	GaSwarmEntity( const BcMat4d& Projection );
	virtual ~GaSwarmEntity();

	virtual void update( BcReal Tick );
	virtual void render( ScnCanvasRef Canvas );

private:
	BcVec2d Position_;

};

#endif
