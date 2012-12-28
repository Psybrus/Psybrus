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

	virtual void						update( BcF32 Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
	eEvtReturn							onKeyboardEvent( EvtID ID, const OsEventInputKeyboard& Event );
	eEvtReturn							onMouseEvent( EvtID ID, const OsEventInputMouse& Event );
	eEvtReturn							onReset( EvtID ID, const GaWorldResetEvent& Event );

	BcVec3d								doShot( const BcVec3d& Direction, BcF32 TrailPower, BcF32 MuzzlePower, BcF32 ImpactPower );
	
public:
	BcBool								MoveForward_;
	BcBool								MoveBackward_;
	BcBool								MoveLeft_;
	BcBool								MoveRight_;
	BcBool								DoRun_;
	BcBool								DoPulse_;
	BcBool								DoShot_;
	BcF32								RateOfShot_;
	BcF32								ShotTick_;

	BcBool								HasWeapon_;
	
	BcF32								Yaw_;
	BcF32								Pitch_;

	BcVec2d								MouseDelta_;

	BcVec3d								LastViewVector_;
	BcVec3d								EarLVectors_[4];
	BcVec3d								EarRVectors_[4];
	
	ScnCanvasComponentRef				Canvas_;
	GaWorldBSPComponentRef				BSP_;
	GaWorldPressureComponentRef			Pressure_;
	GaPawnComponentRef					Pawn_;
};

#endif

