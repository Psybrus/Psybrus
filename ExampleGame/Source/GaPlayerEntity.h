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

#include "GaPhysicsBody.h"

#include "GaBunnyRenderer.h"

////////////////////////////////////////////////////////////////////////////////
// GaPlayerEntity
class GaPlayerEntity:
	public GaEntity
{
public:
	GaPlayerEntity();
	virtual ~GaPlayerEntity();

	virtual void update( BcReal Tick );
	virtual void render( ScnCanvasComponentRef Canvas );

	eEvtReturn onMouseMove( EvtID ID, const OsEventInputMouse& Event );
	eEvtReturn onMouseDown( EvtID ID, const OsEventInputMouse& Event );
	eEvtReturn onMouseUp( EvtID ID, const OsEventInputMouse& Event );

private:
	GaPhysicsBody* pBody_;

	// Movement control.
	BcVec2d TargetPosition_;
	
	GaBunnyRenderer BunnyRenderer_;
};

#endif
