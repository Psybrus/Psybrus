/**************************************************************************
*
* File:		GaPlayerEntity.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Player.
*		
*
*
* 
**************************************************************************/

#ifndef __GaPlayerEntity_H__
#define __GaPlayerEntity_H__

#include "Psybrus.h"

#include "GaEntity.h"

////////////////////////////////////////////////////////////////////////////////
// GaPlayerEntity
class GaPlayerEntity:
	public GaEntity
{
public:
	GaPlayerEntity( const BcMat4d& Projection );
	virtual ~GaPlayerEntity();

	virtual void update( BcReal Tick );
	virtual void render( ScnCanvasRef Canvas );

	eEvtReturn onMouseMove( EvtID ID, const OsEventInputMouse& Event );
	eEvtReturn onMouseDown( EvtID ID, const OsEventInputMouse& Event );
	eEvtReturn onMouseUp( EvtID ID, const OsEventInputMouse& Event );

private:
	BcMat4d Projection_;

	// Movement control.
	BcVec2d TargetPosition_;
	BcVec2d Velocity_;
	BcVec2d Acceleration_;
	BcReal LerpDelta_;	
	BcReal MovementSpeed_;
	

};

#endif
