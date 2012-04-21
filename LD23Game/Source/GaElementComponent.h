/**************************************************************************
*
* File:		GaElementComponent.h
* Author:	Neil Richardson 
* Ver/Date:	29/12/11	
* Description:
*		Example user component.
*		
*
*
* 
**************************************************************************/

#ifndef __GAELEMENTCOMPONENT_H__
#define __GAELEMENTCOMPONENT_H__

#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef CsResourceRef< class GaElementComponent > GaElementComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaElementComponent
class GaElementComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaElementComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

private:
	BcReal								Rotation_;
	BcVec3d								Position_;
	BcVec3d								Velocity_;

};

#endif
