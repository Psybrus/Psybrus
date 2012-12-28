/**************************************************************************
*
* File:		GaPawnComponent.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		Pawn component.
*		
*
*
* 
**************************************************************************/

#ifndef __GaPawnComponent_H__
#define __GaPawnComponent_H__

#include "Psybrus.h"

#include "GaWorldBSPComponent.h"
#include "GaWorldPressureComponent.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef CsResourceRef< class GaPawnComponent > GaPawnComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaPawnComponent
class GaPawnComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaPawnComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcF32 Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	void								setPosition( const BcVec3d& Position );
	void								setMove( const BcVec3d& MoveDirection );

private:
	BcVec3d								Position_;
	BcVec3d								MoveDirection_;
	
	ScnCanvasComponentRef				Canvas_;
	GaWorldBSPComponentRef				BSP_;
	GaWorldPressureComponentRef			Pressure_;
};

#endif

