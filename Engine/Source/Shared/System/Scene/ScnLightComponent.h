/**************************************************************************
*
* File:		ScnLightComponent.h
* Author:	Neil Richardson 
* Ver/Date:	11/01/13
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnLightComponent_H__
#define __ScnLightComponent_H__

#include "System/Renderer/RsCore.h"
#include "System/Scene/ScnTypes.h"
#include "System/Scene/ScnComponent.h"

//////////////////////////////////////////////////////////////////////////
// ScnLightType
enum ScnLightType
{
	scnLT_POINT,
	scnLT_SPOT
};

//////////////////////////////////////////////////////////////////////////
// ScnLightComponent
class ScnLightComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, ScnLightComponent );

public:
	void								initialise();
	virtual void						initialise( const Json::Value& Object );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	/**
	*	Find attenuation of light by a distance.
	*/
	BcF32								findAttenuationByDistance( BcF32 Distance ) const;

	/**
	*	Find distance from light by an attenuation.
	*/
	BcF32								findDistanceByAttenuation( BcF32 Attenuation ) const;
	
	/**
	*	Create attenuation values using min, mid and max distances.
	*/
	void								createAttenuationValues( BcF32 MinDistance, BcF32 MidDistance, BcF32 MaxDistance );

	/**
	*	Set material parameters.
	*/
	void								setMaterialParameters( BcU32 LightIndex, class ScnMaterialComponent* MaterialComponent );

private:
	class ScnLightManagerComponent*		LightManager_;
	ScnLightType						Type_;
	RsColour							Colour_;
	BcF32								AttnC_;
	BcF32								AttnL_;
	BcF32								AttnQ_;
};

#endif
