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
#include "System/Scene/ScnSpatialComponent.h"

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
	public ScnSpatialComponent
{
public:
	DECLARE_RESOURCE( ScnSpatialComponent, ScnLightComponent );
	DECLARE_VISITABLE( ScnLightComponent );

public:
	void								initialise();
	virtual void						initialise( const Json::Value& Object );
	virtual void						update( BcF32 Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	/**
	*	Get ambient colour.
	*/ 
	const RsColour&						getAmbientColour() const;

	/**
	*	Get diffuse colour.
	*/ 
	const RsColour&						getDiffuseColour() const;

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

	/**
	 * Set the light tree node we are in.
	 */
	void								setLightTreeNode( class ScnLightTreeNode* Node );

	/**
	 * Get the light tree node we are in.
	 */
	class ScnLightTreeNode*				getLightTreeNode();

	/**
	 *	Get light AABB.
	 */
	BcAABB								getAABB() const;

private:
	ScnLightType						Type_;
	RsColour							AmbientColour_;
	RsColour							DiffuseColour_;
	BcF32								AttnC_;
	BcF32								AttnL_;
	BcF32								AttnQ_;
};

#endif
