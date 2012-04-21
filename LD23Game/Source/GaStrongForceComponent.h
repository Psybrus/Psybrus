/**************************************************************************
*
* File:		GaStrongForceComponent.h
* Author:	Neil Richardson 
* Ver/Date:	21/04/12
* Description:
*	
*		
*
*
* 
**************************************************************************/

#ifndef __GASTRONGFORCECOMPONENT_H__
#define __GASTRONGFORCECOMPONENT_H__

#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef CsResourceRef< class GaStrongForceComponent > GaStrongForceComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaStrongForceComponent
class GaStrongForceComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaStrongForceComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	eEvtReturn							onMouseEvent( EvtID ID, const OsEventInputMouse& Event );

private:
	friend class GaGameComponent;
	
	RsColour							Colour_;

	BcVec3d								Position_;

	ScnViewComponentRef					View_;
	ScnModelComponentRef				Model_;
	ScnMaterialComponentRef				Material_;


	BcBool								IsCharging_;
	BcBool								IsActive_;

	BcReal								Radius_;
	BcReal								TargetRadius_;

	BcU32								MaterialColourParam_;
};

#endif
