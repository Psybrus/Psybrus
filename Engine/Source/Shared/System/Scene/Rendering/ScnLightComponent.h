/**************************************************************************
*
* File:		Rendering/ScnLightComponent.h
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
	scnLT_SPOT,

	scnLT_MAX
};

//////////////////////////////////////////////////////////////////////////
// ScnLightComponent
class ScnLightComponent:
	public ScnSpatialComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnLightComponent, ScnSpatialComponent );
	DECLARE_VISITABLE( ScnLightComponent );

public:
	ScnLightComponent();
	virtual ~ScnLightComponent();

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	/**
	 *	Get ambient colour.
	 */ 
	const RsColour& getAmbientColour() const;

	/**
	 *	Get diffuse colour.
	 */ 
	const RsColour& getDiffuseColour() const;

	/**
	 *	Find attenuation of light by a distance.
	 */
	BcF32 findAttenuationByDistance( BcF32 Distance ) const;

	/**
	 *	Find distance from light by an attenuation.
	 */
	BcF32 findDistanceByAttenuation( BcF32 Attenuation ) const;
	
	/**
	 *	Create attenuation values using min, mid and max distances.
	 */
	void createAttenuationValues( BcF32 MinDistance, BcF32 MidDistance, BcF32 MaxDistance );

	/**
	 *	Set light uniform block data.
	 */
	void setLightUniformBlockData( BcU32 LightIndex, struct ScnShaderLightUniformBlockData& OutLightData );

	/**
	 *	Get light AABB.
	 */
	MaAABB getAABB() const override;

private:
	ScnLightType Type_;
	RsColour AmbientColour_;
	RsColour DiffuseColour_;
	BcF32 Min_;
	BcF32 Mid_;
	BcF32 Max_;

	BcF32 AttnC_;
	BcF32 AttnL_;
	BcF32 AttnQ_;
};

#endif
