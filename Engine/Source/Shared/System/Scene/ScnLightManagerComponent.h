/**************************************************************************
*
* File:		ScnLightManagerComponent.h
* Author:	Neil Richardson 
* Ver/Date:	11/01/13
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnLightManagerComponent_H__
#define __ScnLightManagerComponent_H__

#include "System/Renderer/RsCore.h"
#include "System/Scene/ScnTypes.h"
#include "System/Scene/ScnComponent.h"

//////////////////////////////////////////////////////////////////////////
// ScnLightManagerComponent
class ScnLightManagerComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, ScnLightManagerComponent );

public:
	void								initialise();
	virtual void						initialise( const Json::Value& Object );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	virtual void						registerLightComponent( class ScnLightComponent* LightComponent );
	virtual void						unregisterLightComponent( class ScnLightComponent* LightComponent );

	void								setMaterialParameters( class ScnMaterialComponent* MaterialComponent ) const;

private:
	typedef std::vector< class ScnLightComponent* > TLightComponentList;

	BcBool								DummyMemberRemove_;
	TLightComponentList					LightComponents_;

};

#endif
