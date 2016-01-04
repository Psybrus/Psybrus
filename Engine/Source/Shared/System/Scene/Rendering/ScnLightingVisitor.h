/**************************************************************************
*
* File:		Rendering/ScnLightingVisitor.h
* Author: 	Neil Richardson 
* Ver/Date:	13/01/13
* Description:
*		Lighting visitor for the scenegraph.
*		
*
*
* 
**************************************************************************/

#ifndef __ScnLightingVisitor_H__
#define __ScnLightingVisitor_H__

#include "System/Scene/ScnVisitor.h"
#include "System/Scene/Rendering/ScnShaderFileData.h"

//////////////////////////////////////////////////////////////////////////
// ScnLightingVisitor
class ScnLightingVisitor:
	public ScnVisitor
{
public:
	enum
	{
		MAX_LIGHTS = 16
	};

public:
	ScnLightingVisitor( const class MaAABB& AABB );
	virtual ~ScnLightingVisitor();

	virtual void visit( class ScnLightComponent* Component );

	const ScnShaderLightUniformBlockData& getLightUniformBlockData() const { return LightUniformData_; }

private:
	std::array< ScnLightComponent*, MAX_LIGHTS > LightComponents_;
	size_t NoofLights_ = 0;
	ScnShaderLightUniformBlockData LightUniformData_ = {};

};

#endif
