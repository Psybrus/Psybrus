/**************************************************************************
*
* File:		GaEnemyComponent.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		Player component.
*		
*
*
* 
**************************************************************************/

#ifndef __GaEnemyComponent_H__
#define __GaEnemyComponent_H__

#include "Psybrus.h"

#include "GaPawnComponent.h"
#include "GaWorldBSPComponent.h"
#include "GaWorldPressureComponent.h"

//////////////////////////////////////////////////////////////////////////
// GaEnemyComponentRef
typedef CsResourceRef< class GaEnemyComponent > GaEnemyComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaEnemyComponent
class GaEnemyComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaEnemyComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcF32 Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	BcVec3d								findLongestDirection();
	
private:
	ScnCanvasComponentRef				Canvas_;
	GaWorldBSPComponentRef				BSP_;
	GaWorldPressureComponentRef			Pressure_;
	GaPawnComponentRef					Pawn_;

	BcVec3d								Direction_;

	BcF32								PulseTimer_;

	GaPawnComponentRef					PlayerPawn_;

	BcBool								IsTargetting_;
	BcF32								TargetTimer_;


};

#endif

