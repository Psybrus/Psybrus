/**************************************************************************
*
* File:		GaPlayerComponent.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		Player component.
*		
*
*
* 
**************************************************************************/

#ifndef __GaPlayerComponent_H__
#define __GaPlayerComponent_H__

#include "Psybrus.h"

#include "GaPawnComponent.h"
#include "GaWorldBSPComponent.h"
#include "GaWorldPressureComponent.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef CsResourceRef< class GaPlayerComponent > GaPlayerComponentRef;


//////////////////////////////////////////////////////////////////////////
// GaPlayerComponent
class GaPlayerComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaPlayerComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
	eEvtReturn							onKeyboardEvent( EvtID ID, const OsEventInputKeyboard& Event );
	
private:
	BcBool								MoveForward_;
	BcBool								MoveBackward_;
	BcBool								MoveLeft_;
	BcBool								MoveRight_;
	
	BcReal								Rotation_;
	
	ScnCanvasComponentRef				Canvas_;
	GaWorldBSPComponentRef				BSP_;
	GaWorldPressureComponentRef			Pressure_;
	GaPawnComponentRef					Pawn_;
};

#endif

