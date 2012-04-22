/**************************************************************************
*
* File:		GaElementComponent.h
* Author:	Neil Richardson 
* Ver/Date:	21/04/12
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
	friend class GaGameComponent;

	BcName								FuseType_;
	BcName								ReplaceType_;
	BcName								RespawnType_;

	RsColour							Colour_;
	RsColour							ShadowColour_;
	BcReal								MaxSpeed_;
	BcReal								Direction_;
	BcReal								Radius_;

	BcVec3d								AngularVelocity_;
	BcVec3d								Rotation_;

	ScnModelComponentRef				Model_;
	ScnMaterialComponentRef				Material_;
	BcU32								MaterialColourParam_;

	ScnModelComponentRef				ShadowModel_;
	ScnMaterialComponentRef				ShadowMaterial_;
	BcU32								ShadowMaterialColourParam_;


};

#endif
