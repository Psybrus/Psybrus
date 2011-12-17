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
// Forward Declarations
class GaMainGameState;

////////////////////////////////////////////////////////////////////////////////
// GaEntity
class GaEntity
{
public:
	GaEntity();
	virtual ~GaEntity();

	virtual void update( BcReal Tick );
	virtual void render( ScnCanvasRef Canvas );

	BcForceInline void setParent( GaMainGameState* pParent ){ pParent_ = pParent; }
	BcForceInline GaMainGameState* pParent(){ return pParent_; }
	BcForceInline const BcVec2d& getPosition() const{ return Position_; };

protected:
	GaMainGameState* pParent_;
	BcMat4d Projection_;
	BcVec2d Position_;

};

#endif
