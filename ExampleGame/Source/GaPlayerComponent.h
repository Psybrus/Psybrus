/**************************************************************************
*
* File:		GaPlayerComponent.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Player.
*		
*
*
* 
**************************************************************************/

#ifndef __GaPlayerComponent_H__
#define __GaPlayerComponent_H__

#include "Psybrus.h"

#include "GaGameComponent.h"

#include "GaPhysicsBody.h"

#include "GaLayeredSpriteComponent.h"

////////////////////////////////////////////////////////////////////////////////
// Typedefs
typedef CsResourceRef< class GaPlayerComponent > GaPlayerComponentRef;

////////////////////////////////////////////////////////////////////////////////
// GaPlayerComponent
class GaPlayerComponent:
	public GaGameComponent
{
public:
	DECLARE_RESOURCE( GaGameComponent, GaPlayerComponent );

	virtual void initialise();
	virtual void destroy();
	virtual void update( BcReal Tick );
	virtual void render( ScnCanvasComponentRef Canvas );

	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );

	eEvtReturn onMouseMove( EvtID ID, const OsEventInputMouse& Event );
	eEvtReturn onMouseDown( EvtID ID, const OsEventInputMouse& Event );
	eEvtReturn onMouseUp( EvtID ID, const OsEventInputMouse& Event );

private:
	GaPhysicsBody* pBody_;

	// Movement control.
	BcVec2d TargetPosition_;
	
	GaLayeredSpriteComponentRef LayeredSpriteComponent_;
};

#endif
